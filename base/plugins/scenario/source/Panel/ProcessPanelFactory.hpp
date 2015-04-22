#pragma once
#include <iscore/plugins/panel/PanelFactoryInterface.hpp>

class ProcessPanelFactory : public iscore::PanelFactoryInterface
{
    public:
        QString name() const override { return "ProcessPanelModel"; }
        iscore::PanelViewInterface*makeView(iscore::View* parent) override;
        iscore::PanelPresenterInterface*makePresenter(iscore::Presenter* parent_presenter,
                                                      iscore::PanelViewInterface* view) override;
        iscore::PanelModelInterface*makeModel(iscore::DocumentModel* parent) override;
};
