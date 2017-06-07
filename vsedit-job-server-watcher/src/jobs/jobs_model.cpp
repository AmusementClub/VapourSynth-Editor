#include "jobs_model.h"

#include "../../../common-src/settings/settings_manager.h"
#include "../../../common-src/helpers.h"

#include <QGuiApplication>
#include <cassert>

//==============================================================================

const int JobsModel::NAME_COLUMN = 0;
const int JobsModel::TYPE_COLUMN = 1;
const int JobsModel::SUBJECT_COLUMN = 2;
const int JobsModel::STATE_COLUMN = 3;
const int JobsModel::DEPENDS_ON_COLUMN = 4;
const int JobsModel::TIME_START_COLUMN = 5;
const int JobsModel::TIME_END_COLUMN = 6;
const int JobsModel::FPS_COLUMN = 7;
const int JobsModel::COLUMNS_NUMBER = 8;

//==============================================================================

JobsModel::JobsModel(SettingsManager * a_pSettingsManager,
	QObject * a_pParent) :
	  QAbstractItemModel(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
	, m_fpsDisplayPrecision(DEFAULT_FPS_DISPLAY_PRECISION)
{
	assert(m_pSettingsManager);
}

// END OF JobsModel::JobsModel(SettingsManager * a_pSettingsManager,
//		QObject * a_pParent)
//==============================================================================

JobsModel::~JobsModel()
{
}

// END OF JobsModel::~JobsModel()
//==============================================================================

QModelIndex JobsModel::index(int a_row, int a_column,
	const QModelIndex & a_parent) const
{
	(void)a_parent;
	return createIndex(a_row, a_column);
}

// END OF QModelIndex JobsModel::index(int a_row, int a_column,
//		const QModelIndex & a_parent) const
//==============================================================================

QModelIndex JobsModel::parent(const QModelIndex & a_child) const
{
	(void)a_child;
	return QModelIndex();
}

// END OF QModelIndex JobsModel::parent(const QModelIndex & a_child) const
//==============================================================================

Qt::ItemFlags JobsModel::flags(const QModelIndex & a_index) const
{
	if (!a_index.isValid())
		return Qt::NoItemFlags;

	int row = a_index.row();
	int column = a_index.column();

	if((row >= (int)m_jobs.size()) || (column >= COLUMNS_NUMBER))
		return Qt::NoItemFlags;

	Qt::ItemFlags cellFlags = Qt::NoItemFlags
		| Qt::ItemIsEnabled
		| Qt::ItemIsSelectable
	;

	bool modifiable = canModifyJob(row);
	if((a_index.column() == DEPENDS_ON_COLUMN) && modifiable && (row > 0))
		cellFlags |= Qt::ItemIsEditable;

	return cellFlags;
}

// END OF Qt::ItemFlags JobsModel::flags(const QModelIndex & a_index) const
//==============================================================================

QVariant JobsModel::headerData(int a_section, Qt::Orientation a_orientation,
	int a_role) const
{
	if(a_orientation != Qt::Horizontal)
		return QVariant();

	if((a_role != Qt::DisplayRole) && (a_role != Qt::ToolTipRole))
		return QVariant();

	switch(a_section)
	{
	case NAME_COLUMN:
		return trUtf8("Name");
	case TYPE_COLUMN:
		return trUtf8("Type");
	case SUBJECT_COLUMN:
		return trUtf8("Subject");
	case STATE_COLUMN:
		return trUtf8("State");
	case DEPENDS_ON_COLUMN:
		return trUtf8("Depends on jobs");
	case TIME_START_COLUMN:
		return trUtf8("Started");
	case TIME_END_COLUMN:
		return trUtf8("Ended");
	case FPS_COLUMN:
		return trUtf8("FPS");
	default:
		return QVariant();
	}

	return QVariant();
}

// END OF QVariant JobsModel::headerData(int a_section,
//		Qt::Orientation a_orientation, int a_role) const
//==============================================================================

QVariant JobsModel::data(const QModelIndex & a_index, int a_role) const
{
	if(!a_index.isValid())
		return QVariant();

	int row = a_index.row();
	int column = a_index.column();

	if((a_index.row() >= (int)m_jobs.size()) ||
		(a_index.column() >= COLUMNS_NUMBER))
		return QVariant();

	const QString dateTimeFormat = "yyyy-MM-dd\nhh:mm:ss.z";

	if((a_role == Qt::DisplayRole) || (a_role == Qt::ToolTipRole))
	{
		if(column == NAME_COLUMN)
			return trUtf8("Job %1").arg(row + 1);
		else if(column == TYPE_COLUMN)
			return JobProperties::typeName(m_jobs[row].type);
		else if(column == SUBJECT_COLUMN)
			return m_jobs[row].subject();
		else if(column == STATE_COLUMN)
			return JobProperties::stateName(m_jobs[row].jobState);
		else if(column == DEPENDS_ON_COLUMN)
		{
			QStringList dependsList;
			for(const QUuid & id : m_jobs[row].dependsOnJobIds)
			{
				ptrdiff_t index = indexOfJob(id);
				if(index < 0)
					dependsList << trUtf8("<invalid job>");
				else
					dependsList << trUtf8("Job %1").arg(index + 1);
			}
			return dependsList.join(", ");
		}
		else if(column == TIME_START_COLUMN)
		{
			QDateTime timeStarted = m_jobs[row].timeStarted;
			if(timeStarted != QDateTime())
				return timeStarted.toLocalTime().toString(dateTimeFormat);
		}
		else if(column == TIME_END_COLUMN)
		{
			QDateTime timeStarted = m_jobs[row].timeEnded;
			if(timeStarted != QDateTime())
				return timeStarted.toLocalTime().toString(dateTimeFormat);
		}
		else if((column == FPS_COLUMN) &&
			(m_jobs[row].type == JobType::EncodeScriptCLI) &&
			(m_jobs[row].framesProcessed > 0))
		{
			QString fps = QString::number(m_jobs[row].fps, 'f',
				m_fpsDisplayPrecision);
			int framesTotal = m_jobs[row].framesTotal();
			if(vsedit::contains(ACTIVE_JOB_STATES, m_jobs[row].jobState) &&
				(m_jobs[row].framesProcessed < framesTotal))
			{
				int framesLeft = framesTotal - m_jobs[row].framesProcessed;
				double secondsToFinish = (double)framesLeft / m_jobs[row].fps;
				fps += "\n";
				fps += vsedit::timeToString(secondsToFinish);
			}
			return fps;
		}
	}
	else if(a_role == Qt::TextAlignmentRole)
	{
		const int centeredColumns[] = {STATE_COLUMN, TIME_START_COLUMN,
			TIME_END_COLUMN};
		if(vsedit::contains(centeredColumns, column))
			return Qt::AlignCenter;
	}

	return QVariant();
}

// END OF QVariant JobsModel::data(const QModelIndex & a_index, int a_role)
//		const
//==============================================================================

int JobsModel::rowCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return (int)m_jobs.size();
}

// END OF int JobsModel::rowCount(const QModelIndex & a_parent) const
//==============================================================================

int JobsModel::columnCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return COLUMNS_NUMBER;
}

// END OF int JobsModel::columnCount(const QModelIndex & a_parent) const
//==============================================================================

bool JobsModel::setData(const QModelIndex & a_index, const QVariant & a_value,
	int a_role)
{
	(void)a_role;

	if(!a_index.isValid())
		return false;

	int row = a_index.row();
	int column = a_index.column();

	if((row >= (int)m_jobs.size()) || (column != DEPENDS_ON_COLUMN))
		return false;

	if(!a_value.canConvert<QVariantList>())
		return false;

	QVariantList variantList = a_value.toList();
	std::vector<QUuid> ids;
	for(const QVariant & variant : variantList)
	{
		if(!variant.canConvert<QUuid>())
			return false;
		QUuid id = variant.toUuid();
		int index = indexOfJob(id);
		if((index < 0) || (index > row))
			return false;
		ids.push_back(id);
	}

	emit signalSetDependencies(m_jobs[row].id, ids);

	return true;
}

// END OF bool JobsModel::setData(const QModelIndex & a_index,
//		const QVariant & a_value, int a_role)
//==============================================================================

std::vector<JobProperties> JobsModel::jobs() const
{
	return m_jobs;
}

// END OF std::vector<JobProperties> JobsModel::jobs() const
//==============================================================================

bool JobsModel::setJobs(const std::vector<JobProperties> & a_jobs)
{
	beginResetModel();
	m_jobs = a_jobs;
	endResetModel();
	return true;
}

// END OF bool JobsModel::setJobs(const std::vector<JobProperties> & a_jobs)
//==============================================================================

JobProperties JobsModel::jobProperties(int a_index) const
{
	if((a_index < 0) || ((size_t)a_index >= m_jobs.size()))
		return JobProperties();
	return m_jobs[a_index];
}

// END OF JobProperties JobsModel::jobProperties(int a_index) const
//==============================================================================

int JobsModel::createJob(const JobProperties & a_jobProperties)
{
	int newRow = (int)m_jobs.size();
	beginInsertRows(QModelIndex(), newRow, newRow);
	m_jobs.push_back(a_jobProperties);
	endInsertRows();
	return newRow;
}

// END OF int JobsModel::createJob(const JobProperties & a_jobProperties)
//==============================================================================

bool JobsModel::swapJobs(const QUuid & a_id1, const QUuid & a_id2)
{
	int index1 = indexOfJob(a_id1);
	if(index1 < 0)
		return false;

	int index2 = indexOfJob(a_id2);
	if(index2 < 0)
		return false;

	std::swap(m_jobs[index1], m_jobs[index2]);

	notifyJobUpdated(index1);
	notifyJobUpdated(index2);

	return true;
}

// END OF bool JobsModel::swapJobs(const QUuid & a_id1, const QUuid & a_id2)
//==============================================================================

bool JobsModel::deleteJobs(std::vector<QUuid> a_ids)
{
	for(const QUuid & id : a_ids)
	{
		int index = indexOfJob(id);
		if(index < 0)
			continue;

		beginRemoveRows(QModelIndex(), index, index);
		m_jobs.erase(m_jobs.begin() + index);
		endRemoveRows();
	}

	return true;
}

// END OF bool JobsModel::deleteJobs(std::vector<QUuid> a_ids)
//==============================================================================

bool JobsModel::updateJobProperties(const JobProperties & a_jobProperties)
{
	int index = indexOfJob(a_jobProperties.id);
	if(index < 0)
		return false;
	m_jobs[index] = a_jobProperties;
	notifyJobUpdated(index);
	return true;
}

// END OF bool JobsModel::updateJobProperties(
//		const JobProperties & a_jobProperties)
//==============================================================================

bool JobsModel::setJobDependsOnIds(const QUuid & a_id,
	const std::vector<QUuid> & a_ids)
{
	int index = indexOfJob(a_id);
	if(index < 0)
		return false;
	m_jobs[index].dependsOnJobIds = a_ids;
	notifyJobUpdated(index, DEPENDS_ON_COLUMN);
	return true;
}

// END OF bool JobsModel::setJobDependsOnIds(const QUuid & a_id,
//		const std::vector<QUuid> & a_ids)
//==============================================================================

void JobsModel::requestJobDependsOnIds(const QUuid & a_id,
	const std::vector<QUuid> & a_ids)
{
	emit signalSetDependencies(a_id, a_ids);
}

// END OF void JobsModel::requestJobDependsOnIds(const QUuid & a_id,
//		const std::vector<QUuid> & a_ids)
//==============================================================================

bool JobsModel::setJobProgress(const QUuid & a_id, int a_progress, double a_fps)
{
	int index = indexOfJob(a_id);
	if(index < 0)
		return false;
	m_jobs[index].framesProcessed = a_progress;
	m_jobs[index].fps = a_fps;
	notifyJobUpdated(index, STATE_COLUMN);
	notifyJobUpdated(index, FPS_COLUMN);
	return true;
}

// END OF bool JobsModel::setJobProgress(const QUuid & a_id, int a_progress,
//		double a_fps)
//==============================================================================

bool JobsModel::setJobState(const QUuid & a_id, JobState a_state)
{
	int index = indexOfJob(a_id);
	if(index < 0)
		return false;
	m_jobs[index].jobState = a_state;
	notifyJobUpdated(index, STATE_COLUMN);
	return true;
}

// END OF bool JobsModel::setJobState(const QUuid & a_id, JobState a_state)
//==============================================================================

bool JobsModel::canModifyJob(int a_index) const
{
	if((a_index < 0) || ((size_t)a_index >= m_jobs.size()))
		return false;

	JobState forbiddenStates[] = {JobState::Running, JobState::Paused,
		JobState::Aborting};
	if(vsedit::contains(forbiddenStates, m_jobs[a_index].jobState))
		return false;

	return true;
}

// END OF bool JobsModel::canModifyJob(int a_index) const
//==============================================================================

bool JobsModel::hasActiveJobs()
{
	for(const JobProperties & properties : m_jobs)
	{
		if(vsedit::contains(ACTIVE_JOB_STATES, properties.jobState))
			return true;
	}
	return false;
}

// END OF bool JobsModel::hasActiveJobs()
//==============================================================================

std::vector<int> JobsModel::indexesFromSelection(
	const QItemSelection & a_selection)
{
	std::set<int> indexesSet;
	QModelIndexList modelIndexList = a_selection.indexes();
	for(const QModelIndex & jobIndex : modelIndexList)
		indexesSet.insert(jobIndex.row());
	std::vector<int> indexesVector;
	std::copy(indexesSet.begin(), indexesSet.end(),
		std::back_inserter(indexesVector));
	return indexesVector;
}

// END OF std::vector<int> JobsModel::indexesFromSelection(
//		const QItemSelection & a_selection)
//==============================================================================

std::vector<QUuid> JobsModel::idsFromSelection(
	const QItemSelection & a_selection)
{
	std::vector<int> indexesVector = indexesFromSelection(a_selection);
	std::vector<QUuid> idsVector;
	for(int jobIndex : indexesVector)
		idsVector.push_back(m_jobs[jobIndex].id);
	return idsVector;
}

// END OF std::vector<QUuid> JobsModel::idsFromSelection(
//		const QItemSelection & a_selection)
//==============================================================================

int JobsModel::indexOfJob(const QUuid & a_uuid) const
{
	std::vector<JobProperties>::const_iterator it =
		std::find_if(m_jobs.cbegin(), m_jobs.cend(),
			[&](const JobProperties & a_properties)->bool
			{
				return (a_properties.id == a_uuid);
			});

	return (it == m_jobs.cend()) ?
		-1 : std::distance(m_jobs.cbegin(), it);
}

// END OF int JobsModel::indexOfJob(const QUuid & a_uuid) const
//==============================================================================

void JobsModel::notifyJobUpdated(int a_index, int a_column)
{
	QModelIndex first;
	QModelIndex last;

	if(a_column < 0)
	{
		first = createIndex(a_index, 0);
		last = createIndex(a_index, COLUMNS_NUMBER - 1);
	}
	else
	{
		first = createIndex(a_index, a_column);
		last = createIndex(a_index, a_column);
	}

	emit dataChanged(first, last);
}

// END OF void JobsModel::noifyJobUpdated(int a_index)
//==============================================================================

void JobsModel::notifyState(int a_index)
{
	if((a_index < 0) || ((size_t)a_index >= m_jobs.size()))
		return;

	int progress = 0;
	int progressTotal = 0;
	if(m_jobs[a_index].type == JobType::EncodeScriptCLI)
	{
		progress = m_jobs[a_index].framesProcessed;
		progressTotal = m_jobs[a_index].framesTotal();
	}

	int jobNumber = a_index + 1;
	if(!hasActiveJobs())
		jobNumber = -1;

	emit signalStateChanged(jobNumber, (int)m_jobs.size(),
		m_jobs[a_index].jobState, progress, progressTotal);
}

// END OF void JobsModel::notifyState(int a_index)
//==============================================================================
