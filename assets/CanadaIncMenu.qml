import bb.cascades 1.0

MenuDefinition
{
    property string projectName
    property alias settings: settingsActionItem
    property alias help: helpActionItem
    property bool allowDonations: false
    property string bbWorldID
    property bool showServiceLogging: false
    property bool showSubmitLogs: false
    
    function launchPage(page)
    {
        if (parent.activePane) {
            parent.activePane.push(page);
        } else if (parent.activeTab) {
            parent.activeTab.content.push(page);
        } else {
            parent.push(page);
        }
    }
    
    onCreationCompleted: {
        if (allowDonations) {
            var donator = donateDefinition.createObject();
            addAction(donator);
        }
        
        if (bbWorldID.length > 0) {
            var reviewer = reviewDefinition.createObject();
            addAction(reviewer);
        }
    }
    
    settingsAction: SettingsActionItem
    {
        id: settingsActionItem
        property variant settingsPage
        
        onTriggered:
        {
            console.log("UserEvent: SettingsPage");
            
            if (!settingsPage) {
                definition.source = "SettingsPage.qml"
                settingsPage = definition.createObject()
            }
            
            launchPage(settingsPage);
        }
    }
    
    actions: [
        ActionItem {
            property variant bugReportPage
            title: qsTr("Bug Reports") + Retranslate.onLanguageChanged
            imageSource: "images/ic_bugs.png"
            
            onTriggered: {
                console.log("UserEvent: BugReportPage");
                
                if (!bugReportPage) {
                    definition.source = "BugReportPage.qml"
                    bugReportPage = definition.createObject()
                }
                
                bugReportPage.projectName = projectName;
                bugReportPage.showServiceLogging = showServiceLogging;
                bugReportPage.showSubmitLogs = showSubmitLogs;
                launchPage(bugReportPage);
            }
        }
    ]
    
    helpAction: HelpActionItem
    {
        id: helpActionItem
        property variant helpPage
        
        onTriggered:
        {
            console.log("UserEvent: HelpPage");
            
            if (!helpPage) {
                definition.source = "HelpPage.qml"
                helpPage = definition.createObject()
            }
            
            launchPage(helpPage);
        }
    }
    
    attachedObjects: [
        ComponentDefinition {
            id: definition
        },
        
        ComponentDefinition
        {
            id: donateDefinition
            
            ActionItem
            {
                title: qsTr("Donate") + Retranslate.onLanguageChanged
                imageSource: "images/ic_donate.png"
                
                onTriggered: {
                    console.log("UserEvent: Donate");
                    persist.donate();
                }
            }
        },
        
        ComponentDefinition
        {
            id: reviewDefinition
            
            ActionItem
            {
                title: qsTr("Review") + Retranslate.onLanguageChanged
                imageSource: "images/ic_review.png"
                
                onTriggered: {
                    console.log("UserEvent: ReviewApp");
                    persist.reviewApp();
                }
            }
        },
        
        Invocation
        {
            query {
                invokeTargetId: "sys.bbm.channels.card.previewer"
                uri: "bbmc:C0034D28B"
            }
            
            onArmed: {
                if ( !persist.contains("promoted") ) {
                    trigger("bb.action.OPENBBMCHANNEL");
                    persist.saveValueFor("promoted", 1, false);
                }
            }
        }
    ]
}