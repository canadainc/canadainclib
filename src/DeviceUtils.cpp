#include "bbndk.h"
#include "DeviceUtils.h"
#include "AppLogFetcher.h"
#include "Logger.h"

#include <bb/cascades/ActionItem>
#include <bb/cascades/Application>
#include <bb/cascades/ListView>
#include <bb/cascades/Page>
#include <bb/cascades/QmlDocument>

#include <bb/device/DisplayInfo>
#include <bb/device/HardwareInfo>

#define CLEANUP_FUNC "cleanUp"

namespace canadainc {

using namespace bb::cascades;
using namespace bb::device;

DeviceUtils::DeviceUtils(QObject* parent) :
        QObject(parent), m_hw(NULL), m_display(NULL), m_factor(0)
{
    QDeclarativeContext* rootContext = QmlDocument::defaultDeclarativeEngine()->rootContext();
    rootContext->setContextProperty("deviceUtils", this);
}


QSize DeviceUtils::pixelSize()
{
    if (!m_display) {
        m_display = new DisplayInfo(this);
    }

    return m_display->pixelSize();
}


void DeviceUtils::attachTopBottomKeys(bb::cascades::Page* page, bb::cascades::ListView* listView, bool onBar)
{
    if ( !isPhysicalKeyboardDevice() )
    {
        AbstractActionItem* top = ActionItem::create().title( tr("Top") ).imageSource( QUrl("asset:///images/common/ic_top.png") ).onTriggered( this, SLOT( onTopTriggered() ) );
        connect( top, SIGNAL( destroyed(QObject*) ), this, SLOT( onDestroyed(QObject*) ) );

        AbstractActionItem* bottom = ActionItem::create().title( tr("Bottom") ).imageSource( QUrl("asset:///images/common/ic_bottom.png") ).onTriggered( this, SLOT( onBottomTriggered() ) );
        connect( bottom, SIGNAL( destroyed(QObject*) ), this, SLOT( onDestroyed(QObject*) ) );

        page->addAction(top, onBar ? ActionBarPlacement::OnBar : ActionBarPlacement::Default);
        page->addAction(bottom, onBar ? ActionBarPlacement::OnBar : ActionBarPlacement::Default);

        m_actionToList[top] = listView;
        m_actionToList[bottom] = listView;
    }

    listView->setScrollRole(ScrollRole::Main);
}


bool DeviceUtils::isPhysicalKeyboardDevice()
{
    if (!m_hw) {
        m_hw = new HardwareInfo(this);
    }

    return m_hw->isPhysicalKeyboardDevice();
}


void DeviceUtils::onDestroyed(QObject* obj) {
    m_actionToList.remove(obj);
}


void DeviceUtils::processDirection(ScrollPosition::Type p)
{
    QObject* l = m_actionToList.value( sender() );

    if (l)
    {
        ListView* listView = static_cast<ListView*>(l);
        listView->scrollToPosition(p, ScrollAnimation::None);
    }
}


void DeviceUtils::onBottomTriggered()
{
    LOGGER("UserEvent: BottomTriggered");
    processDirection(ScrollPosition::End);

    AppLogFetcher::getInstance()->record("BottomTriggered");
}


void DeviceUtils::onTopTriggered()
{
    LOGGER("UserEvent: TopTriggered");
    processDirection(ScrollPosition::Beginning);

    AppLogFetcher::getInstance()->record("TopTriggered");
}


bool DeviceUtils::isEqual(bb::cascades::Page* p1, bb::cascades::Page* p2) {
    return p1 == p2;
}


void DeviceUtils::cleanUpAndDestroy(QObject* q)
{
    QMetaObject::invokeMethod(q, CLEANUP_FUNC, Qt::QueuedConnection);
    q->deleteLater();
}


void DeviceUtils::registerTutorialTips(QObject* parent)
{
    QmlDocument* qml = QmlDocument::create("asset:///TutorialTip.qml").parent(parent);
    QmlDocument::defaultDeclarativeEngine()->rootContext()->setContextProperty( "tutorial", qml->createRootObject<QObject>() );
}


void DeviceUtils::log(QVariant const& message, QObject* q)
{
    if (!q) {
        if ( message.type() == QVariant::String ) {
            LOGGER( message.toString() );
        } else if ( message.type() == QVariant::Double ) {
            LOGGER( message.toReal() );
        } else if ( message.type() == QVariant::Int ) {
            LOGGER( message.toLongLong() );
        } else {
            LOGGER(message);
        }
    } else {
        if ( message.type() == QVariant::String ) {
            LOGGER( q << message.toString() );
        } else if ( message.type() == QVariant::Double ) {
            LOGGER( q << message.toReal() );
        } else if ( message.type() == QVariant::Int ) {
            LOGGER( q << message.toLongLong() );
        } else {
            LOGGER(q << message);
        }
    }
}


qreal DeviceUtils::du(qreal units)
{
#if BBNDK_VERSION_AT_LEAST(10,3,0)
    if ( Application::instance()->scene() ) {
        return Application::instance()->scene()->ui()->du(units);
    }
#endif

    if (m_factor) {
        return m_factor*units;
    }

    QSize ps = pixelSize();

    if (ps.width() == 720 && ps.height() == 720) { // n-series
        m_factor = 9;
    } else if (ps.width() == 720 && ps.height() == 1280) { // a-series
        m_factor = 8;
    } else if (ps.width() == 1440 && ps.height() == 1440) { // windmere
        m_factor = 12;
    } else {
        m_factor = 10;
    }

    return m_factor*units;

}


DeviceUtils::~DeviceUtils()
{
}

} /* namespace canadainc */
