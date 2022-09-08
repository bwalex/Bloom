#include "Insight.hpp"

#include <QTimer>
#include <QFontDatabase>

#include "src/Helpers/Paths.hpp"
#include "src/Logger/Logger.hpp"
#include "UserInterfaces/InsightWindow/BloomProxyStyle.hpp"

#include "src/Application.hpp"

#include "InsightWorker/Tasks/QueryLatestVersionNumber.hpp"
#include "InsightWorker/Tasks/GetTargetState.hpp"
#include "InsightWorker/Tasks/GetTargetDescriptor.hpp"

namespace Bloom
{
    using namespace Bloom::Exceptions;
    using Bloom::Targets::TargetState;

    Insight::Insight(
        EventListener& eventListener,
        const ProjectConfig& projectConfig,
        const EnvironmentConfig& environmentConfig,
        const InsightConfig& insightConfig,
        InsightProjectSettings& insightProjectSettings
    )
        : eventListener(eventListener)
        , projectConfig(projectConfig)
        , environmentConfig(environmentConfig)
        , insightConfig(insightConfig)
        , insightProjectSettings(insightProjectSettings)
        , application(
            (
                QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true),
#ifndef BLOOM_DEBUG_BUILD
                QCoreApplication::addLibraryPath(QString::fromStdString(Paths::applicationDirPath() + "/plugins")),
#endif
                QApplication(this->qtApplicationArgc, this->qtApplicationArgv.data())
            )
        )
    {}

    void Insight::run() {
        try {
            this->startup();

            this->workerThread->start();
            this->setThreadState(ThreadState::READY);
            Logger::info("Insight ready");
            this->application.exec();

        } catch (const Exception& exception) {
            Logger::error("Insight encountered a fatal error. See below for errors:");
            Logger::error(exception.getMessage());

        } catch (const std::exception& exception) {
            Logger::error("Insight encountered a fatal error. See below for errors:");
            Logger::error(std::string(exception.what()));
        }

        this->shutdown();
    }

    void Insight::startup() {
        Logger::info("Starting Insight");
        this->setThreadState(ThreadState::STARTING);

        this->eventListener.registerCallbackForEventType<Events::TargetControllerStateChanged>(
            std::bind(&Insight::onTargetControllerStateChangedEvent, this, std::placeholders::_1)
        );

        this->eventListener.registerCallbackForEventType<Events::TargetExecutionStopped>(
            std::bind(&Insight::onTargetStoppedEvent, this, std::placeholders::_1)
        );

        this->eventListener.registerCallbackForEventType<Events::TargetExecutionResumed>(
            std::bind(&Insight::onTargetResumedEvent, this, std::placeholders::_1)
        );

        this->eventListener.registerCallbackForEventType<Events::TargetReset>(
            std::bind(&Insight::onTargetResetEvent, this, std::placeholders::_1)
        );

        this->eventListener.registerCallbackForEventType<Events::RegistersWrittenToTarget>(
            std::bind(&Insight::onTargetRegistersWrittenEvent, this, std::placeholders::_1)
        );

        this->eventListener.registerCallbackForEventType<Events::ProgrammingModeEnabled>(
            std::bind(&Insight::onProgrammingModeEnabledEvent, this, std::placeholders::_1)
        );

        this->eventListener.registerCallbackForEventType<Events::ProgrammingModeDisabled>(
            std::bind(&Insight::onProgrammingModeDisabledEvent, this, std::placeholders::_1)
        );

        QApplication::setQuitOnLastWindowClosed(true);
        QApplication::setStyle(new BloomProxyStyle());

        auto globalStylesheet = QFile(
            QString::fromStdString(
                Paths::compiledResourcesPath() + "/src/Insight/UserInterfaces/InsightWindow/Stylesheets/Global.qss"
            )
        );

        if (!globalStylesheet.open(QFile::ReadOnly)) {
            throw Exception("Failed to open global stylesheet file");
        }

        this->application.setStyleSheet(globalStylesheet.readAll());

        qRegisterMetaType<Bloom::Targets::TargetDescriptor>();
        qRegisterMetaType<Bloom::Targets::TargetPinDescriptor>();
        qRegisterMetaType<Bloom::Targets::TargetPinState>();
        qRegisterMetaType<Bloom::Targets::TargetState>();
        qRegisterMetaType<std::map<int, Bloom::Targets::TargetPinState>>();

        // Load Ubuntu fonts
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-B.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-BI.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-C.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-L.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-LI.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-M.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-MI.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/UbuntuMono-B.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/UbuntuMono-BI.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/UbuntuMono-R.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/UbuntuMono-RI.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-R.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-RI.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-Th.ttf")
        );

        /*
         * We can't run our own event loop here - we have to use Qt's event loop. But we still need to be able to
         * process our events. To address this, we use a QTimer to dispatch our events on an interval.
         *
         * This allows us to use Qt's event loop whilst still being able to process our own events.
         */
        auto* eventDispatchTimer = new QTimer(&(this->application));
        QObject::connect(eventDispatchTimer, &QTimer::timeout, this, &Insight::dispatchEvents);
        eventDispatchTimer->start(50);

        QObject::connect(
            this->mainWindow,
            &InsightWindow::activatedSignal,
            this,
            &Insight::onInsightWindowActivated
        );

        this->mainWindow->setInsightConfig(this->insightConfig);
        this->mainWindow->setEnvironmentConfig(this->environmentConfig);

        // Prepare worker thread
        this->workerThread = new QThread();
        this->workerThread->setObjectName("IW" + QString::number(this->insightWorker->id));
        this->insightWorker->moveToThread(this->workerThread);
        QObject::connect(this->workerThread, &QThread::started, this->insightWorker, &InsightWorker::startup);
        QObject::connect(this->workerThread, &QThread::finished, this->insightWorker, &QObject::deleteLater);
        QObject::connect(this->workerThread, &QThread::finished, this->workerThread, &QThread::deleteLater);

        this->mainWindow->init(this->targetControllerConsole.getTargetDescriptor());

        QObject::connect(this->insightWorker, &InsightWorker::ready, this, [this] {
            this->checkBloomVersion();
        });

        this->mainWindow->show();
    }

    void Insight::shutdown() {
        if (this->getThreadState() == ThreadState::STOPPED) {
            return;
        }

        Logger::info("Shutting down Insight");

        this->mainWindow->close();

        if (this->workerThread != nullptr && this->workerThread->isRunning()) {
            Logger::debug("Stopping InsightWorker thread");
            this->workerThread->quit();
            Logger::debug("Waiting for InsightWorker thread to stop");
            this->workerThread->wait();
        }

        this->application.exit(0);
        this->setThreadState(ThreadState::STOPPED);
    }

    void Insight::checkBloomVersion() {
        auto currentVersionNumber = Application::VERSION;

        auto* versionQueryTask = new QueryLatestVersionNumber(
            currentVersionNumber
        );

        QObject::connect(
            versionQueryTask,
            &QueryLatestVersionNumber::latestVersionNumberRetrieved,
            this,
            [this, currentVersionNumber] (const VersionNumber& latestVersionNumber) {
                if (latestVersionNumber > currentVersionNumber) {
                    Logger::warning(
                        "Bloom v" + latestVersionNumber.toString()
                            + " is available to download - upgrade via " + Paths::homeDomainName()
                    );
                }
            }
        );

        this->insightWorker->queueTask(versionQueryTask);
    }

    void Insight::onInsightWindowActivated() {
        auto* getTargetStateTask = new GetTargetState();
        QObject::connect(
            getTargetStateTask,
            &GetTargetState::targetState,
            this,
            [this] (Targets::TargetState targetState) {
                this->lastTargetState = targetState;
                emit this->insightSignals->targetStateUpdated(this->lastTargetState);
            }
        );

        InsightWorker::queueTask(getTargetStateTask);
    }

    void Insight::onTargetStoppedEvent(const Events::TargetExecutionStopped& event) {
        if (this->lastTargetState == TargetState::STOPPED) {
            return;
        }

        this->lastTargetState = TargetState::STOPPED;
        emit this->insightSignals->targetStateUpdated(TargetState::STOPPED);
    }

    void Insight::onTargetResumedEvent(const Events::TargetExecutionResumed& event) {
        if (this->lastTargetState != TargetState::RUNNING) {
            this->lastTargetState = TargetState::RUNNING;
            emit this->insightSignals->targetStateUpdated(TargetState::RUNNING);
        }
    }

    void Insight::onTargetResetEvent(const Events::TargetReset& event) {
        try {
            if (this->targetControllerConsole.getTargetState() != TargetState::STOPPED) {
                return;
            }

            if (this->lastTargetState != TargetState::STOPPED) {
                this->lastTargetState = TargetState::STOPPED;
                emit this->insightSignals->targetStateUpdated(TargetState::STOPPED);
            }

            emit this->insightSignals->targetReset();

        } catch (const Exceptions::Exception& exception) {
            Logger::debug("Error handling TargetReset event - " + exception.getMessage());
        }
    }

    void Insight::onTargetRegistersWrittenEvent(const Events::RegistersWrittenToTarget& event) {
        emit this->insightSignals->targetRegistersWritten(event.registers, event.createdTimestamp);
    }

    void Insight::onTargetControllerStateChangedEvent(const Events::TargetControllerStateChanged& event) {
        using TargetController::TargetControllerState;

        if (event.state == TargetControllerState::SUSPENDED) {
            emit this->insightSignals->targetControllerSuspended();

        } else if (event.state == TargetControllerState::ACTIVE) {
            auto* getTargetDescriptorTask = new GetTargetDescriptor();

            QObject::connect(
                getTargetDescriptorTask,
                &GetTargetDescriptor::targetDescriptor,
                this,
                [this] (Targets::TargetDescriptor targetDescriptor) {
                    emit this->insightSignals->targetControllerResumed(targetDescriptor);
                }
            );

            InsightWorker::queueTask(getTargetDescriptorTask);
        }
    }

    void Insight::onProgrammingModeEnabledEvent(const Events::ProgrammingModeEnabled& event) {
        emit this->insightSignals->programmingModeEnabled();
    }

    void Insight::onProgrammingModeDisabledEvent(const Events::ProgrammingModeDisabled& event) {
        emit this->insightSignals->programmingModeDisabled();
    }
}
