#include "MoveStates.hpp"
#include "Process/Temporal/StateMachines/ScenarioStateMachine.hpp"
#include <Process/ScenarioModel.hpp>

#include "Commands/Scenario/Displacement/MoveEvent.hpp"
#include "Commands/Scenario/Displacement/MoveTimeNode.hpp"
#include "Commands/Scenario/Displacement/MoveConstraint.hpp"

#include <QFinalState>

MoveConstraintState::MoveConstraintState(const ScenarioStateMachine& stateMachine,
                                         ObjectPath&& scenarioPath,
                                         iscore::CommandStack& stack,
                                         iscore::ObjectLocker& locker,
                                         QState* parent):
    CommonScenarioState{ObjectPath{scenarioPath}, parent},
    m_dispatcher{std::move(scenarioPath), locker, stack}
{
    using namespace Scenario::Command ;
    auto finalState = new QFinalState{this};

    QState* mainState = new QState{this};
    {
        QState* pressed = new QState{mainState};
        QState* released = new QState{mainState};
        QState* moving = new QState{mainState};

        // General setup
        mainState->setInitialState(pressed);
        released->addTransition(finalState);

        auto t_pressed =
                make_transition<MoveOnAnything_Transition>(
                    pressed, moving , *this);
        connect(t_pressed, &QAbstractTransition::triggered, [&] ()
        {
            auto scenar = m_scenarioPath.find<ScenarioModel>();
            m_constraintInitialStartDate= scenar->constraint(clickedConstraint)->startDate();
            m_constraintInitialClickDate = currentPoint.date;
        });

        make_transition<ReleaseOnAnything_Transition>(
                    pressed, finalState);
        make_transition<MoveOnAnything_Transition>(
                    moving , moving , *this);
        make_transition<ReleaseOnAnything_Transition>(
                    moving , released);

        QObject::connect(moving, &QState::entered, [&] ()
        {
            m_dispatcher.submitCommand(
                        new MoveConstraint{
                            ObjectPath{m_scenarioPath},
                            clickedConstraint,
                            m_constraintInitialStartDate + (currentPoint.date - m_constraintInitialClickDate),
                            currentPoint.y,
                            stateMachine.expandMode()});
        });

        QObject::connect(released, &QState::entered, [&] ()
        {
            m_dispatcher.commit();
        });
    }

    QState* rollbackState = new QState{this};
    make_transition<Cancel_Transition>(mainState, rollbackState);
    rollbackState->addTransition(finalState);
    QObject::connect(rollbackState, &QState::entered, [&] ()
    {
        m_dispatcher.rollback();
    });

    setInitialState(mainState);
}


MoveEventState::MoveEventState(const ScenarioStateMachine& stateMachine,
                               ObjectPath&& scenarioPath,
                               iscore::CommandStack& stack,
                               iscore::ObjectLocker& locker,
                               QState* parent):
    CommonScenarioState{ObjectPath{scenarioPath}, parent},
    m_dispatcher{std::move(scenarioPath), locker, stack}
{
    using namespace Scenario::Command ;
    auto finalState = new QFinalState{this};

    QState* mainState = new QState{this};
    {
        QState* pressed = new QState{mainState};
        QState* released = new QState{mainState};
        QState* moving = new QState{mainState};

        // General setup
        mainState->setInitialState(pressed);
        released->addTransition(finalState);

        make_transition<MoveOnAnything_Transition>(
                    pressed, moving, *this);
        make_transition<ReleaseOnAnything_Transition>(
                    pressed, finalState);
        make_transition<MoveOnAnything_Transition>(
                    moving, moving, *this);
        make_transition<ReleaseOnAnything_Transition>(
                    moving, released);

        // What happens in each state.
        QObject::connect(moving, &QState::entered, [&] ()
        {
            m_dispatcher.submitCommand(
                        new MoveEvent{
                            ObjectPath{m_scenarioPath},
                            clickedEvent,
                            currentPoint.date,
                            currentPoint.y,
                            stateMachine.expandMode()});
        });

        QObject::connect(released, &QState::entered, [&] ()
        {
            m_dispatcher.commit();
        });
    }

    QState* rollbackState = new QState{this};
    make_transition<Cancel_Transition>(mainState, rollbackState);
    rollbackState->addTransition(finalState);
    QObject::connect(rollbackState, &QState::entered, [&] ()
    {
        m_dispatcher.rollback();
    });

    setInitialState(mainState);
}


MoveTimeNodeState::MoveTimeNodeState(const ScenarioStateMachine &stateMachine,
                                     ObjectPath&& scenarioPath,
                                     iscore::CommandStack& stack,
                                     iscore::ObjectLocker& locker,
                                     QState* parent):
    CommonScenarioState{ObjectPath{scenarioPath}, parent},
    m_dispatcher{std::move(scenarioPath), locker, stack}
{
    using namespace Scenario::Command ;
    auto finalState = new QFinalState{this};

    QState* mainState = new QState{this};
    {
        QState* pressed = new QState{mainState};
        QState* released = new QState{mainState};
        QState* moving = new QState{mainState};
        mainState->setInitialState(pressed);

        // General setup
        released->addTransition(finalState);

        make_transition<MoveOnAnything_Transition>(
                    pressed, moving, *this);
        make_transition<ReleaseOnAnything_Transition>(
                    pressed, finalState);
        make_transition<MoveOnAnything_Transition>(
                    moving, moving, *this);
        make_transition<ReleaseOnAnything_Transition>(
                    moving, released);

        // What happens in each state.
        QObject::connect(moving, &QState::entered, [&] ()
        {
            // Get the 1st event on the timenode.
            auto scenar = m_scenarioPath.find<ScenarioModel>();
            auto tn = scenar->timeNode(clickedTimeNode);
            auto ev_id = tn->events().first();


            m_dispatcher.submitCommand(
                        new MoveTimeNode{
                            ObjectPath{m_scenarioPath},
                            ev_id,
                            currentPoint.date,
                            scenar->event(ev_id)->heightPercentage(),
                            stateMachine.expandMode()});
        });

        QObject::connect(released, &QState::entered, [&] ()
        {
            m_dispatcher.commit();
        });
    }

    QState* rollbackState = new QState{this};
    make_transition<Cancel_Transition>(mainState, rollbackState);
    rollbackState->addTransition(finalState);
    QObject::connect(rollbackState, &QState::entered, [&] ()
    {
        m_dispatcher.rollback();
    });

    setInitialState(mainState);
}
