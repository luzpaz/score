#include <Process/LayerModel.hpp>
#include <Process/ProcessList.hpp>
#include <Process/Style/ScenarioStyle.hpp>
#include <Scenario/Document/Constraint/Rack/RackModel.hpp>
#include <Scenario/Document/Constraint/Rack/Slot/SlotModel.hpp>
#include <Scenario/Document/Constraint/ViewModels/FullView/FullViewConstraintViewModel.hpp>
#include <iscore/document/DocumentInterface.hpp>
#include <iscore/application/ApplicationContext.hpp>
#include <map>
#include <utility>

#include "ConstraintModel.hpp"
#include <Process/Process.hpp>
#include <Process/TimeValue.hpp>
#include <Scenario/Document/Constraint/ConstraintDurations.hpp>
#include <Scenario/Document/Constraint/ViewModels/ConstraintViewModel.hpp>
#include <iscore/model/ModelMetadata.hpp>
#include <iscore/tools/Todo.hpp>

namespace Scenario
{
class StateModel;
class TimeNodeModel;

ConstraintModel::ConstraintModel(
    const Id<ConstraintModel>& id,
    const Id<ConstraintViewModel>& fullViewId,
    double yPos,
    QObject* parent)
    : Entity{id, Metadata<ObjectKey_k, ConstraintModel>::get(), parent}
    , m_fullViewModel{new FullViewConstraintViewModel{fullViewId, *this, this}}
{
  initConnections();
  setupConstraintViewModel(m_fullViewModel);
  metadata().setInstanceName(*this);
  metadata().setColor(ScenarioStyle::instance().ConstraintDefaultBackground);
  setHeightPercentage(yPos);
}

ConstraintModel::~ConstraintModel()
{
  static_assert(std::is_same<serialization_tag<ConstraintModel>::type, visitor_entity_tag>::value, "");
}

ConstraintModel::ConstraintModel(
    const ConstraintModel& source,
    const Id<ConstraintModel>& id,
    QObject* parent)
    : Entity{source, id, Metadata<ObjectKey_k, ConstraintModel>::get(), parent}
{
  metadata().setInstanceName(*this);
  initConnections();
  // It is not necessary to save modelconsistency because it should be
  // recomputed

  m_startState = source.startState();
  m_endState = source.endState();
  duration = source.duration;

  m_startDate = source.m_startDate;
  m_heightPercentage = source.heightPercentage();

  // For an explanation of this, see ReplaceConstraintContent command
  std::map<const Process::ProcessModel*, Process::ProcessModel*> processPairs;

  // Clone the processes
  for (const auto& process : source.processes)
  {
    auto newproc = process.clone(process.id(), this);

    processPairs.insert(std::make_pair(&process, newproc));
    processes.add(newproc);

    // We don't need to resize them since the new constraint will have the same
    // duration.
  }

  auto& procs
      = iscore::AppComponents().interfaces<Process::LayerFactoryList>();
  for (const auto& rack : source.racks)
  {
    racks.add(new RackModel{
        rack, rack.id(),
        [&](const SlotModel& source_slot, SlotModel& target) {
          for (auto& lm : source_slot.layers)
          {
            // We can safely reuse the same id since it's in a different slot.
            auto proc = processPairs[&lm.processModel()];
            auto fact = procs.findDefaultFactory(proc->concreteKey());
            // TODO harmonize the order of parameters (source first, then new
            // id)
            target.layers.add(fact->cloneLayer(*proc, lm.id(), lm, &target));
          }
        },
        this});
  }

  // NOTE : we do not copy the view models on which this constraint does not
  // have ownership,
  // this is the job of a command.
  // However, the full view constraint must be copied since we have ownership
  // of it.

  m_fullViewModel
      = source.fullView()->clone(source.fullView()->id(), *this, this);
}

void ConstraintModel::setupConstraintViewModel(ConstraintViewModel* viewmodel)
{
  racks.removing
      .connect<ConstraintViewModel, &ConstraintViewModel::on_rackRemoval>(
          viewmodel);

  connect(
      viewmodel, &ConstraintViewModel::about_to_be_deleted, this,
      &ConstraintModel::on_destroyedViewModel);

  m_constraintViewModels.push_back(viewmodel);
  emit viewModelCreated(*viewmodel);
}

void ConstraintModel::on_destroyedViewModel(ConstraintViewModel* obj)
{
  int index = m_constraintViewModels.indexOf(obj);

  if (index != -1)
  {
    m_constraintViewModels.remove(index);
    emit viewModelRemoved(obj);
  }
}

void ConstraintModel::initConnections()
{
  racks.added.connect<ConstraintModel, &ConstraintModel::on_rackAdded>(this);
}

void ConstraintModel::on_rackAdded(const RackModel& rack)
{
  processes.removed
      .connect<RackModel, &RackModel::on_deleteSharedProcessModel>(
          const_cast<RackModel&>(rack));
  con(duration, &ConstraintDurations::defaultDurationChanged, &rack,
      &RackModel::on_durationChanged);
}

const Id<StateModel>& ConstraintModel::startState() const
{
  return m_startState;
}

void ConstraintModel::setStartState(const Id<StateModel>& e)
{
  m_startState = e;
}

const Id<StateModel>& ConstraintModel::endState() const
{
  return m_endState;
}

void ConstraintModel::setEndState(const Id<StateModel>& endState)
{
  m_endState = endState;
}

const TimeVal& ConstraintModel::startDate() const
{
  return m_startDate;
}

void ConstraintModel::setStartDate(const TimeVal& start)
{
  m_startDate = start;
  emit startDateChanged(start);
}

void ConstraintModel::translate(const TimeVal& deltaTime)
{
  setStartDate(m_startDate + deltaTime);
}

// Simple getters and setters

double ConstraintModel::heightPercentage() const
{
  return m_heightPercentage;
}

void ConstraintModel::setFullView(FullViewConstraintViewModel* fv)
{
  m_fullViewModel = fv;
  setupConstraintViewModel(m_fullViewModel);
}

// Should go in an "execution" object.
void ConstraintModel::startExecution()
{
  for (Process::ProcessModel& proc : processes)
  {
    proc.startExecution(); // prevents editing
  }
}
void ConstraintModel::stopExecution()
{
  duration.setPlayPercentage(0);
  duration.setExecutionSpeed(1.0);
  for (Process::ProcessModel& proc : processes)
  {
    proc.stopExecution();
  }
}

void ConstraintModel::reset()
{
  duration.setPlayPercentage(0);
  duration.setExecutionSpeed(1.0);

  for (Process::ProcessModel& proc : processes)
  {
    proc.reset();
    proc.stopExecution();
  }

  setExecutionState(ConstraintExecutionState::Enabled);
}

void ConstraintModel::setHeightPercentage(double arg)
{
  if (m_heightPercentage != arg)
  {
    m_heightPercentage = arg;
    emit heightPercentageChanged(arg);
  }
}

void ConstraintModel::setExecutionState(ConstraintExecutionState s)
{
  if (s != m_executionState)
  {
    m_executionState = s;
    emit executionStateChanged(s);
  }
}
}
