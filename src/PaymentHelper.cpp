#include "PaymentHelper.h"
#include "AppLogFetcher.h"
#include "Persistance.h"
#include "Logger.h"

#include <bb/platform/ExistingPurchasesReply>
#include <bb/platform/PaymentManager>
#include <bb/platform/PurchaseReply>

namespace canadainc {

using namespace bb::platform;

PaymentHelper::PaymentHelper(Persistance* persist, QObject* parent) :
        QObject(parent), m_persistance(persist), m_payment(NULL)
{
    if ( !persist->containsFlag("purchasesRefreshed") )
    {
        connect( this, SIGNAL( initialize() ), this, SLOT( refreshPurchases() ), Qt::QueuedConnection ); // async startup
        emit initialize();
    }
}


PaymentManager* PaymentHelper::getPaymentManager()
{
    if (!m_payment)
    {
        m_payment = new PaymentManager(this);
        connect( m_payment, SIGNAL( existingPurchasesFinished(bb::platform::ExistingPurchasesReply*) ), this, SLOT( existingPurchasesFinished(bb::platform::ExistingPurchasesReply*) ) );
        connect( m_payment, SIGNAL( purchaseFinished(bb::platform::PurchaseReply*) ), this, SLOT( purchaseFinished(bb::platform::PurchaseReply*) ) );

#if !defined(QT_NO_DEBUG) || DEBUG_RELEASE
        m_payment->setConnectionMode(PaymentConnectionMode::Test);
#endif
    }

    return m_payment;
}


void PaymentHelper::existingPurchasesFinished(bb::platform::ExistingPurchasesReply* reply)
{
    QList<PurchaseReceipt> purchases = reply->purchases();
    LOGGER( purchases.size() );

    for (int i = purchases.size()-1; i >= 0; i--)
    {
        PurchaseReceipt p = purchases[i];

        if ( p.state() == DigitalGoodState::Owned ) {
            m_persistance->saveValueFor( p.digitalGoodSku(), p.date() );
        }
    }

    reply->deleteLater();
    m_persistance->setFlag("purchasesRefreshed", 1);
    m_persistance->showToast( tr("Purchases successfully refreshed!"), "images/common/restored_purchases.png" );
}


void PaymentHelper::purchaseFinished(bb::platform::PurchaseReply* reply)
{
    LOGGER( reply->purchaseMetadata() );
    PurchaseReceipt p = reply->receipt();

    QString sku = p.digitalGoodSku();

    if ( !sku.isEmpty() )
    {
        m_persistance->saveValueFor( p.digitalGoodSku(), p.date() );
        AppLogFetcher::getInstance()->record("Purchased", sku);
        m_persistance->showToast( tr("Succesfully purchased!"), "images/common/purchased.png" );
    }

    reply->deleteLater();
}


void PaymentHelper::refreshPurchases()
{
    LOGGER("Requesting");
    getPaymentManager()->requestExistingPurchases();
}


void PaymentHelper::requestPurchase(QString const& sku, QString const& name)
{
    LOGGER(sku << name);
    getPaymentManager()->requestPurchase("", sku, name);

    AppLogFetcher::getInstance()->record("RequestPurchase", sku);
}


PaymentHelper::~PaymentHelper()
{
}

} /* namespace canadainc */
