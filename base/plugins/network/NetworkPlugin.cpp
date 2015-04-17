#include <NetworkPlugin.hpp>
#include <NetworkControl.hpp>
#include <settings_impl/NetworkSettings.hpp>
#include "DistributedScenario/Panel/GroupPanelView.hpp"

#define PROCESS_NAME "Network Process"

NetworkPlugin::NetworkPlugin() :
    QObject {},
        iscore::PluginControlInterface_QtInterface {},
iscore::SettingsDelegateFactoryInterface_QtInterface {}
{
    setObjectName("NetworkPlugin");
}

// Interfaces implementations :
//////////////////////////
iscore::SettingsDelegateFactoryInterface* NetworkPlugin::settings_make()
{
    return new NetworkSettings;
}

iscore::PluginControlInterface* NetworkPlugin::control_make()
{
    return new NetworkControl;
}

iscore::PanelFactoryInterface *NetworkPlugin::panel_make(QString name)
{
    if(name == "GroupPanel")
    {
        return new GroupPanelFactory;
    }
}

