#pragma once
#include <iscore/tools/SettableIdentifier.hpp>
#include <iscore/command/SerializableCommand.hpp>
class AbstractConstraintViewModel;

#include <iscore/tools/ObjectPath.hpp>

class EventModel;

namespace Scenario
{
    namespace Command
    {
        /**
        * @brief The RemoveEvent class
        *
        * remove an event
        *
        */
        class RemoveEvent : public iscore::SerializableCommand
        {
                ISCORE_COMMAND
            public:
                ISCORE_COMMAND_DEFAULT_CTOR(RemoveEvent, "ScenarioControl")
                RemoveEvent(ObjectPath&& scenarioPath, EventModel* event);
                virtual void undo() override;
                virtual void redo() override;
                virtual bool mergeWith(const Command* other) override;

            protected:
                virtual void serializeImpl(QDataStream&) const override;
                virtual void deserializeImpl(QDataStream&) override;

            private:
                ObjectPath m_path;
                id_type<EventModel> m_evId {};
                QByteArray m_serializedEvent;
                QByteArray m_serializedTimeNode;
                QVector<QByteArray> m_serializedConstraints; // The handlers inside the events are IN the constraints / Boxes / etc.

                QMap<std::tuple<int, int, int>, id_type<AbstractConstraintViewModel>> m_constraintViewModelIDs;
        };
    }
}
