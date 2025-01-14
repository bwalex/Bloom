## TargetController

The TargetController component possesses full control of the connected hardware (debug tool and target). Execution of
user-space device drivers takes place here. All interactions with the connected hardware go through the
TargetController. It runs on a dedicated thread (see `Applciation::startTargetController()`). The source code for the
TargetController component can be found in src/TargetController. The entry point is `TargetControllerComponent::run()`.

### Interfacing with the TargetController - The command-response mechanism

Other components within Bloom can interface with the TargetController via the provided command-response mechanism.
Simply put, when another component within Bloom needs to interact with the connected hardware, it will send a command to
the TargetController, and wait for a response. The TargetController will action the command and deliver the necessary
response.

All TargetController commands can be found in [src/TargetController/Commands](./Commands), and are derived from the
[`TargetController::Commands::Command`](./Commands/Command.hpp) base class. Responses can be found in
[src/TargetController/Responses](./Responses), and are derived from the
[`TargetController::Responses::Response`](./Responses/Response.hpp) base class.

**NOTE:** Components within Bloom do not typically concern themselves with the TargetController command-response
mechanism. Instead, they use the `TargetControllerService` class, which encapsulates the command-response mechanism and
provides a simplified means for interaction with the connected hardware. For more, see
[The TargetControllerService class](#the-TargetControllerService-class) section below.

Commands can be sent to the TargetController via the [`TargetController::CommandManager`](./CommandManager.hpp)
class.

For example, to read memory from the connected target, we would send the
[`TargetController::Commands::ReadTargetMemory`](./Commands/ReadTargetMemory.hpp) command:

```c++
auto tcCommandManager = TargetController::CommandManager();

auto readMemoryCommand = std::make_unique<TargetController::Commands::ReadTargetMemory>(
    someMemoryType, // Flash, RAM, EEPROM, etc
    someStartAddress,
    someNumberOfBytes
);

const auto readMemoryResponse = tcCommandManager.sendCommandAndWaitForResponse(
    std::move(readMemoryCommand),
    std::chrono::milliseconds(1000) // Response timeout
);

const auto& data = readMemoryResponse->data;
```

`readMemoryResponse` will be of type `std::unique_ptr<TargetController::Responses::TargetMemoryRead>`.

The `CommandManager::sendCommandAndWaitForResponse<CommandType>(std::unique_ptr<CommandType> command, std::chrono::milliseconds timeout)`
member function is a template function. It will issue the command to the TargetController and wait for a response, or
until a timeout has been reached. Because it is a template function, it is able to resolve the appropriate response
type at compile-time (see the `SuccessResponseType` alias in some command classes). If the TargetController responds
with an error, or the timeout is reached, `CommandManager::sendCommandAndWaitForResponse()` will throw an exception.

#### Atomic operations

In some instances, we need the TargetController to service a series of commands without any interruptions (servicing of
other commands).

The TargetController allows for operations to be performed within "atomic sessions". Simply put, when the
TargetController starts a new atomic session, any commands that are part of the session will be placed into a dedicated
queue. When an atomic session is active, the TargetController will only process commands in the dedicated queue.
All other commands will be processed once the atomic session has ended.

The `TargetControllerService` provides an RAII wrapper for atomic sessions. See
[Atomic sessions with the TargetControllerService](#atomic-sessions-with-the-TargetControllerService) for more.

#### The TargetControllerService class

The `TargetControllerService` class encapsulates the TargetController's command-response mechanism and provides a
simplified means for other components to interact with the connected hardware. Iterating on the example above, to read
memory from the target:

```c++
const auto tcService = Services::TargetControllerService();

const auto data = tcService.readMemory(
    someMemoryType, // Flash, RAM, EEPROM, etc
    someStartAddress,
    someNumberOfBytes
);
```

The `TargetControllerService` class does not require any dependencies at construction. It can be constructed in
different threads and used freely to gain access to the connected hardware, from any component within Bloom.

All components within Bloom should use the `TargetControllerService` class to interact with the connected hardware. They
**should not** directly issue commands via the `TargetController::CommandManager`, unless there is a very good
reason to do so.

##### Atomic sessions with the TargetControllerService

The `TargetControllerService::makeAtomicSession()` member function returns an `TargetControllerService::AtomicSession`
RAII object, which starts an atomic session with the TargetController, at construction, and ends the session at
destruction. This allows us to perform operations within an atomic session, in an exception-safe manner:

```c++
auto tcService = Services::TargetControllerService();

{
    const auto atomicSession = tcService.makeAtomicSession();

    /*
     * These operations will take place in the atomic session - the TC will **NOT** service any other commands until
     * these commands have been processed (and the atomic session has ended).
     */
    tcService.writeMemory(...);
    tcService.readMemory(...);
    tcService.getProgramCounter();

    /*
     * Note: The TC does **NOT** support nested atomic sessions. Attempting to start another session in this block will
     * result in an exception being thrown.
     */
    {
        const auto nestedAtomicSession = tcService.makeAtomicSession(); // This will fail - a session is already active.
    }

    /*
     * Also note: When using TargetControllerService::makeAtomicSession(), the returned AtomicSession object is tied to
     * the TargetControllerService object that created it (in this example: tcService).
     *
     * So if you have **another** TargetControllerService object in this block, any operations performed via that
     * object will **NOT** be part of the atomic session, and, they will deadlock the TC. So don't ever do this.
     * You should never need more than one TargetControllerService object in a single block.
     */

    // Don't ever do this.
    auto anotherTcService = Services::TargetControllerService();

    // These operations will **NOT** be part of the atomic session, and they will cause a deadlock and timeout.
    anotherTcService.writeMemory(...);
    anotherTcService.readMemory(...);

    /*
     * One more thing: The AtomicSession object should **NEVER** outlive the TargetControllerService object that
     * created it.
     *
     * If this happens, the AtomicSession will have a dangling reference, which will result in UB.
     */
}

// At this point, the atomic session will have ended. The TC will now process any other commands in the queue.
tcService.readMemory(...); // Will not be part of the atomic session
```

### Programming mode

When a component needs to write to the target's program memory, it must enable programming mode on the target. This can
be done by issuing the `EnableProgrammingMode` command to the TargetController (see
`TargetControllerService::enableProgrammingMode()`).

Once programming mode has been enabled, standard debugging operations such as program flow control and RAM access will
become unavailable. The TargetController will reject any commands that involve these operations, until programming mode
has been disabled. The [`Command::requiresDebugMode()`](./Commands/Command.hpp) virtual member function communicates a
particular command's requirement for the target to **not** be in programming mode.

For example, the `ResumeTargetExecution` command returns `true` here, as it attempts to control program flow on the
target, which can only be done when the target is not in programming mode:

```c++
class ResumeTargetExecution: public Command
{
public:
    // ...
    [[nodiscard]] bool requiresDebugMode() const override {
        return true;
    }
};
```

On the other hand, the `ReadTargetMemory` command will only return `true` if we're reading from RAM, as RAM is the
only memory which isn't accessible when the target is in programming mode:

```c++
class ReadTargetMemory: public Command
{
public:
    Targets::TargetMemoryType memoryType;
    Targets::TargetMemoryAddress startAddress;
    Targets::TargetMemorySize bytes;

    // ...

    [[nodiscard]] bool requiresDebugMode() const override {
        return this->memoryType == Targets::TargetMemoryType::RAM;
    }
};
```

The TargetController will emit `ProgrammingModeEnabled` and `ProgrammingModeDisabled` events when it enables/disables
programming mode. Components should listen for these events to ensure that they disable any means for the user to trigger
debugging operations whilst programming mode is enabled. For example, the Insight component will disable much of its
GUI components when programming mode is enabled.

It shouldn't be too much of a problem if a component attempts to perform a debug operation on the target whilst
programming mode is enabled, as the TargetController will just respond with an error. But still, it would be best to
avoid doing this where possible.

---

TODO: Cover debug tool & target drivers.
