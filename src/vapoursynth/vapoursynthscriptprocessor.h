#ifndef VAPOURSYNTHSCRIPTPROCESSOR_H
#define VAPOURSYNTHSCRIPTPROCESSOR_H

#include "vs_script_processor_structures.h"
#include "../settings/settingsmanager.h"

#include <QObject>
#include <QLibrary>
#include <vapoursynth/VSScript.h>
#include <deque>
#include <vector>
#include <map>

//==============================================================================

typedef int (VS_CC *FNP_vssInit)(void);
typedef const VSAPI * (VS_CC *FNP_vssGetVSApi)(void);
typedef int (VS_CC *FNP_vssEvaluateScript)(VSScript ** a_handle,
	const char * a_script, const char * a_scriptFilename, int a_flags);
typedef const char * (VS_CC *FNP_vssGetError)(VSScript * a_handle);
typedef VSCore * (VS_CC *FNP_vssGetCore)(VSScript * a_handle);
typedef VSNodeRef * (VS_CC *FNP_vssGetOutput)(VSScript * a_handle,
	int a_index);
typedef void (VS_CC *FNP_vssFreeScript)(VSScript * a_handle);
typedef int (VS_CC *FNP_vssFinalize)(void);

//==============================================================================

class VapourSynthScriptProcessor : public QObject
{
	Q_OBJECT

public:

	VapourSynthScriptProcessor(SettingsManager * a_pSettingsManager,
		QObject * a_pParent = nullptr);

	virtual ~VapourSynthScriptProcessor();

	bool initialize(const QString& a_script, const QString& a_scriptName);

	bool finalize();

	bool isInitialized() const;

	QString error() const;

	const VSVideoInfo * videoInfo(int a_outputIndex = 0);

	const VSAPI * api() const;

	const VSFrameRef * requestFrame(int a_frameNumber,
		int a_outputIndex = 0);

	bool requestFrameAsync(int a_frameNumber, int a_outputIndex = 0,
		bool a_needPreview = false);

	bool flushFrameTicketsQueue();

public slots:

	void slotResetSettings();

signals:

	void signalWriteLogMessage(int a_messageType,
		const QString & a_message);

	void signalDistributeFrame(int a_frameNumber, int a_outputIndex,
		const VSFrameRef * a_cpOutputFrameRef,
		const VSFrameRef * a_cpPreviewFrameRef);

	void signalFrameQueueStateChanged(size_t a_inQueue, size_t a_inProcess,
		size_t a_maxThreads);

private slots:

	void slotReceiveFrameAndProcessQueue(
		const VSFrameRef * a_cpFrameRef, int a_frameNumber,
		VSNodeRef * a_pNodeRef, QString a_errorMessage);

private:

	void handleVSMessage(int a_messageType, const QString & a_message);

	void receiveFrame(const VSFrameRef * a_cpFrameRef, int a_frameNumber,
		VSNodeRef * a_pNodeRef, const QString & a_errorMessage);

	bool initLibrary();

	void freeLibrary();

	void processFrameTicketsQueue();

	void sendFrameQueueChangeSignal();

	bool recreatePreviewNode(NodePair & a_nodePair);

	void freeFrameTicket(FrameTicket & a_ticket);

	NodePair getNodePair(int a_outputIndex, bool a_needPreview);

	friend void VS_CC vsMessageHandler(int a_msgType,
		const char * a_message, void * a_pUserData);

	SettingsManager * m_pSettingsManager;

	QString m_script;

	QString m_scriptName;

	QString m_error;

	bool m_vsScriptInitialized;

	bool m_initialized;
	const VSAPI * m_cpVSAPI;

	VSScript * m_pVSScript;

	const VSVideoInfo * m_cpVideoInfo;
	const VSCoreInfo * m_cpCoreInfo;

	QLibrary m_vsScriptLibrary;

	FNP_vssInit vssInit;
	FNP_vssGetVSApi vssGetVSApi;
	FNP_vssEvaluateScript vssEvaluateScript;
	FNP_vssGetError vssGetError;
	FNP_vssGetCore vssGetCore;
	FNP_vssGetOutput vssGetOutput;
	FNP_vssFreeScript vssFreeScript;
	FNP_vssFinalize vssFinalize;

	std::deque<FrameTicket> m_frameTicketsQueue;
	std::vector<FrameTicket> m_frameTicketsInProcess;
	std::map<int, NodePair> m_nodePairForOutputIndex;

	ResamplingFilter m_chromaResamplingFilter;
	ChromaPlacement m_chromaPlacement;
	double m_resamplingFilterParameterA;
	double m_resamplingFilterParameterB;
	YuvMatrixCoefficients m_yuvMatrix;
};

//==============================================================================

#endif // VAPOURSYNTHSCRIPTPROCESSOR_H
