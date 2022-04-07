import QtQuick 2.3
import QtQuick.Controls 1.2 as QtControls
import QtQuick.Layouts 1.1
import Oxygen 1.0
import Oxygen.Dialogs 1.0
import Oxygen.Layouts 1.0
import Oxygen.Themes 1.0
import Oxygen.Tools 1.0
import Oxygen.Widgets 1.0


Item{
    id: root
    property string configFile: ""
    property var enableControls: true

    function queryProperties()
    {
        var props = {}
            props["MQTT_PLUGIN/ConfigFile"] = root.configFile
                return props;
            }

            ColumnLayout{
                anchors.leftMargin: Theme.smallMargin
                anchors.rightMargin: Theme.smallMargin
                anchors.fill: parent

                spacing: Theme.mediumSpacing

                RowLayout{
                    Label{
                        text: "Path to MQTT-Config File"
                    }

                    TextField{
                        id: idConfigFile
                        readOnly: false
                        enabled: root.enableControls

                        onActiveFocusChanged: {
                            if (!activeFocus) {
                            root.configFile = text
                        }
                    }
                }

                Button{
                    id: configBrowseButton
                    text: "Browse" + Theme.actionEllipsis
                    onClicked:
                {
                    fileDialog.open();
                }
            }
        }
        VerticalSpacer { }
    }
    FileDialog {
        id: fileDialog

        selectExisting: true
        nameFilters: ["Configuration file (*.json)"]
        title: "Open MQTT configuration file"

        lastFolderKey: "MQTT_PLUGIN/ConfigFolder"

        onAccepted: {
            idConfigFile.text = Utilities.urlToLocalFile(fileUrl);
            root.configFile = idConfigFile.text
        }
    }
}
