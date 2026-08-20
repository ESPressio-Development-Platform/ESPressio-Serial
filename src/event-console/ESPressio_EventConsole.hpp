#pragma once

#if !__has_include(<ESPressio_EventTransport.hpp>)
#error "ESPressio EventConsole requires ESPressio Event >= 5.6.1."
#endif

#if !__has_include(<ESPressio_JsonArchive.hpp>)
#error "ESPressio EventConsole requires ESPressio Serializable >= 0.9.0 < 1.0.0 with the optional JSON adapter and ArduinoJson."
#endif

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

#include <Arduino.h>
#include <ESPressio_EventTransport.hpp>
#include <ESPressio_JsonArchive.hpp>
#include <ESPressio_Command.hpp>

#include "../command-console/ESPressio_CommandConsole.hpp"

#include "../console/ESPressio_Console.hpp"
#include "../logging/ESPressio_ILoggerSink.hpp"
#include "ESPressio_EventConsoleTypes.hpp"

namespace ESPressio::Serial {

class EventConsole final {
private:
    enum class InteractionState : uint8_t {
        None,
        AwaitingJson,
        AwaitingConfirmation
    };

    Console* _console = nullptr;
    Print* _output = nullptr;

    Event::EventTransportManager*
        _manager = nullptr;

    EventConsoleConfig _config;

    std::unordered_set<std::string>
        _allowList;

    std::unordered_set<std::string>
        _denyList;

    ILoggerSink* _auditSink = nullptr;

    uint32_t _interceptorID = 0;

    Command::CommandRegistrationHandle _eventCommandRegistration;
    Command::CommandRegistrationHandle _eventsCommandRegistration;
    bool _commandBacked = false;

    InteractionState _state =
        InteractionState::None;

    std::string _pendingTypeName;

    Event::EventDispatchMethod
        _pendingMethod =
            Event::EventDispatchMethod::Queue;

    Event::EventPriority
        _pendingPriority =
            Event::EventPriority::Normal;

    Event::EventTransportDirection
        _pendingDefaultDirection =
            Event::EventTransportDirection::None;

    std::unique_ptr<Event::IEvent>
        _pendingEvent;

    mutable std::mutex _mutex;


    static std::string_view Trim(
        std::string_view value
    ) noexcept {
        while (
            !value.empty() &&
            std::isspace(
                static_cast<unsigned char>(
                    value.front()
                )
            )
        ) {
            value.remove_prefix(1);
        }

        while (
            !value.empty() &&
            std::isspace(
                static_cast<unsigned char>(
                    value.back()
                )
            )
        ) {
            value.remove_suffix(1);
        }

        return value;
    }


    static std::pair<
        std::string_view,
        std::string_view
    >
    SplitFirst(
        std::string_view value
    ) noexcept {
        value = Trim(value);

        const auto separator =
            value.find_first_of(
                " \t"
            );

        if (
            separator ==
            std::string_view::npos
        ) {
            return {
                value,
                {}
            };
        }

        return {
            value.substr(
                0,
                separator
            ),
            Trim(
                value.substr(
                    separator + 1
                )
            )
        };
    }


    static bool EqualsIgnoreCase(
        std::string_view left,
        std::string_view right
    ) noexcept {
        if (
            left.size() !=
            right.size()
        ) {
            return false;
        }

        for (
            std::size_t index = 0;
            index < left.size();
            ++index
        ) {
            if (
                std::tolower(
                    static_cast<unsigned char>(
                        left[index]
                    )
                ) !=
                std::tolower(
                    static_cast<unsigned char>(
                        right[index]
                    )
                )
            ) {
                return false;
            }
        }

        return true;
    }


    static const char*
    DirectionName(
        Event::EventTransportDirection
            direction
    ) noexcept {
        using Direction =
            Event::EventTransportDirection;

        switch (direction) {
            case Direction::None:
                return "None";

            case Direction::Inbound:
                return "Inbound";

            case Direction::Outbound:
                return "Outbound";

            case Direction::Bidirectional:
                return "Bidirectional";
        }

        return "Unknown";
    }


    static const char*
    DispatchMethodName(
        Event::EventDispatchMethod
            method
    ) noexcept {
        return
            method ==
                Event::EventDispatchMethod::
                    Stack
                ? "Stack"
                : "Queue";
    }


    static const char*
    PriorityName(
        Event::EventPriority priority
    ) noexcept {
        switch (priority) {
            case Event::EventPriority::Low:
                return "Low";

            case Event::EventPriority::Normal:
                return "Normal";

            case Event::EventPriority::High:
                return "High";
        }

        return "Unknown";
    }


    static const char*
    ErrorCodeName(
        Serializable::SerializationErrorCode
            code
    ) noexcept {
        using Code =
            Serializable::
                SerializationErrorCode;

        switch (code) {
            case Code::None:
                return "None";

            case Code::MissingRequiredProperty:
                return
                    "MissingRequiredProperty";

            case Code::TypeMismatch:
                return "TypeMismatch";

            case Code::NumericOutOfRange:
                return "NumericOutOfRange";

            case Code::ValidationFailed:
                return "ValidationFailed";

            case Code::UnsupportedSchemaVersion:
                return
                    "UnsupportedSchemaVersion";

            case Code::MigrationFailed:
                return "MigrationFailed";

            case Code::MalformedInput:
                return "MalformedInput";

            case Code::StreamError:
                return "StreamError";

            case Code::DuplicateValue:
                return "DuplicateValue";

            case Code::UnknownEnumValue:
                return "UnknownEnumValue";

            case Code::ResourceLimitExceeded:
                return
                    "ResourceLimitExceeded";
        }

        return "Unknown";
    }


    bool IsAllowed(
        std::string_view typeName
    ) const {
        const std::string name(
            typeName
        );

        if (
            _denyList.find(name) !=
            _denyList.end()
        ) {
            return false;
        }

        if (
            _config.AccessPolicy ==
            EventConsoleAccessPolicy::
                AllRegistered
        ) {
            return true;
        }

        return
            _allowList.find(name) !=
            _allowList.end();
    }


    void Audit(
        LogLevel level,
        const char* message
    ) {
        if (
            _auditSink == nullptr
        ) {
            return;
        }

        const LogEntry entry{
            static_cast<uint64_t>(
                millis()
            ),
            level,
            "EventConsole",
            message
        };

        _auditSink->Write(entry);
    }


    void PrintUsage() {
        if (_output == nullptr) {
            return;
        }

        _output->println(
            "Event console commands:"
        );

        _output->println(
            "  events"
        );

        _output->println(
            "  event list"
        );

        _output->println(
            "  event describe <stable-type-name>"
        );

        _output->println(
            "  event queue <stable-type-name> <json>"
        );

        _output->println(
            "  event stack <stable-type-name> <json>"
        );

        _output->println(
            "  event compose <stable-type-name> [queue|stack]"
        );

        _output->println(
            "  event cancel"
        );
    }


    void ListEvents() {
        const auto descriptors =
            _manager->
                GetRegisteredSerializableEvents();

        _output->println(
            "Registered Serializable Events:"
        );

        if (descriptors.empty()) {
            _output->println(
                "  <none>"
            );

            return;
        }

        for (
            const auto& descriptor :
                descriptors
        ) {
            const bool allowed =
                IsAllowed(
                    descriptor.TypeName
                );

            if (
                !allowed &&
                !_config.
                    ShowDeniedEventsInList
            ) {
                continue;
            }

            _output->print("  ");
            _output->print(
                descriptor.
                    TypeName.c_str()
            );

            _output->print(
                descriptor.CanConstruct
                    ? " [constructible]"
                    : " [not-constructible]"
            );

            _output->print(
                allowed
                    ? " [allowed]"
                    : " [denied]"
            );

            _output->print(
                " schema="
            );

            _output->print(
                descriptor.SchemaVersion
            );

            _output->print(
                " defaultRouting="
            );

            _output->println(
                DirectionName(
                    descriptor.
                        DefaultDirection
                )
            );
        }
    }


    void DescribeEvent(
        std::string_view typeName
    ) {
        typeName = Trim(typeName);

        if (typeName.empty()) {
            _output->println(
                "Usage: event describe <stable-type-name>"
            );

            return;
        }

        Event::SerializableEventDescriptor
            descriptor;

        if (
            !_manager->
                FindRegisteredSerializableEvent(
                    typeName,
                    descriptor
                )
        ) {
            _output->println(
                "Event type is not registered."
            );

            return;
        }

        _output->print("Event: ");
        _output->println(
            descriptor.TypeName.c_str()
        );

        _output->print("Type ID: 0x");

        char typeID[24];

        std::snprintf(
            typeID,
            sizeof(typeID),
            "%016llX",
            static_cast<
                unsigned long long
            >(
                descriptor.TypeID
            )
        );

        _output->println(typeID);

        _output->print("Schema: ");
        _output->println(
            descriptor.SchemaVersion
        );

        _output->print(
            "Runtime constructible: "
        );

        _output->println(
            descriptor.CanConstruct
                ? "yes"
                : "no"
        );

        _output->print(
            "Operator access: "
        );

        _output->println(
            IsAllowed(
                descriptor.TypeName
            )
                ? "allowed"
                : "denied"
        );

        _output->print(
            "Default transport direction: "
        );

        _output->println(
            DirectionName(
                descriptor.
                    DefaultDirection
            )
        );

        _output->println(
            "Properties:"
        );

        if (
            descriptor.Properties.empty()
        ) {
            _output->println(
                "  <none>"
            );

            return;
        }

        for (
            const auto& property :
                descriptor.Properties
        ) {
            _output->print("  ");
            _output->print(
                property.Name.c_str()
            );

            _output->print(" : ");
            _output->print(
                property.Type.c_str()
            );

            if (property.Required) {
                _output->print(
                    " required"
                );
            }

            if (property.ReadOnly) {
                _output->print(
                    " read-only"
                );
            }

            if (
                property.Sensitive &&
                _config.
                    ShowSensitivePropertyMetadata
            ) {
                _output->print(
                    " sensitive"
                );
            }

            if (property.HasDefault) {
                _output->print(
                    " default"
                );
            }

            _output->println();

            if (
                _config.ShowAliases &&
                !property.Aliases.empty()
            ) {
                _output->print(
                    "      aliases: "
                );

                for (
                    std::size_t index = 0;
                    index <
                        property.
                            Aliases.size();
                    ++index
                ) {
                    if (index > 0) {
                        _output->print(
                            ", "
                        );
                    }

                    _output->print(
                        property.
                            Aliases[index].
                                c_str()
                    );
                }

                _output->println();
            }
        }
    }


    void PrintConstructionIssues(
        const Serializable::
            DeserializationResult& result
    ) {
        if (result.Success()) {
            return;
        }

        _output->print(
            "Event payload validation failed with "
        );

        _output->print(
            static_cast<unsigned long>(
                result.IssueCount()
            )
        );

        _output->println(
            " issue(s):"
        );

        for (
            const auto& issue :
                result.Issues()
        ) {
            _output->print("  ");

            if (!issue.Path.empty()) {
                _output->print(
                    issue.Path.c_str()
                );

                _output->print(": ");
            }

            _output->print(
                ErrorCodeName(
                    issue.Code
                )
            );

            if (!issue.Message.empty()) {
                _output->print(" - ");
                _output->print(
                    issue.Message.c_str()
                );
            }

            _output->println();
        }
    }


    void CancelPending(
        const char* reason = nullptr
    ) {
        _pendingEvent.reset();
        _pendingTypeName.clear();
        _pendingDefaultDirection =
            Event::EventTransportDirection::None;
        _state =
            InteractionState::None;

        if (
            _output != nullptr &&
            reason != nullptr
        ) {
            _output->println(reason);
        }
    }


    void DispatchPending() {
        if (!_pendingEvent) {
            CancelPending(
                "No pending Event."
            );

            return;
        }

        const auto method =
            _pendingMethod;

        const auto priority =
            _pendingPriority;

        const auto typeName =
            _pendingTypeName;

        auto event =
            std::move(
                _pendingEvent
            );

        _pendingTypeName.clear();
        _pendingDefaultDirection =
            Event::EventTransportDirection::None;
        _state =
            InteractionState::None;

        const auto result =
            Event::
                EventTransportManager::
                    DispatchSerializableEvent(
                        std::move(event),
                        method,
                        priority
                    );

        if (
            result ==
            Event::
                RuntimeEventDispatchResult::
                    Dispatched
        ) {
            _output->print(
                "Event dispatched: "
            );

            _output->print(
                typeName.c_str()
            );

            _output->print(" via ");
            _output->print(
                DispatchMethodName(
                    method
                )
            );

            _output->print(
                " priority="
            );

            _output->println(
                PriorityName(
                    priority
                )
            );

            Audit(
                LogLevel::Info,
                "Operator-dispatched Serializable Event."
            );
        } else {
            _output->println(
                "Event dispatch failed."
            );

            Audit(
                LogLevel::Error,
                "Serializable Event dispatch failed."
            );
        }
    }


    void RequestConfirmation() {
        _state =
            InteractionState::
                AwaitingConfirmation;

        _output->print(
            "Dispatch Event '"
        );

        _output->print(
            _pendingTypeName.c_str()
        );

        _output->print("' via ");

        _output->print(
            DispatchMethodName(
                _pendingMethod
            )
        );

        _output->print(
            " priority="
        );

        _output->print(
            PriorityName(
                _pendingPriority
            )
        );

        _output->print(
            " defaultRouting="
        );

        _output->print(
            DirectionName(
                _pendingDefaultDirection
            )
        );

        _output->println(
            "? [y/N]"
        );
    }


    void ConstructAndDispatch(
        std::string_view typeName,
        std::string_view json,
        Event::EventDispatchMethod method
    ) {
        typeName = Trim(typeName);
        json = Trim(json);

        if (
            typeName.empty() ||
            json.empty()
        ) {
            _output->println(
                "Event type and JSON payload are required."
            );

            return;
        }

        if (
            json.size() >
            _config.MaximumJsonLength
        ) {
            _output->println(
                "JSON payload exceeds configured maximum length."
            );

            Audit(
                LogLevel::Warning,
                "Rejected oversized Event JSON payload."
            );

            return;
        }

        Event::SerializableEventDescriptor
            descriptor;

        if (
            !_manager->
                FindRegisteredSerializableEvent(
                    typeName,
                    descriptor
                )
        ) {
            _output->println(
                "Event type is not registered."
            );

            Audit(
                LogLevel::Warning,
                "Attempted dispatch of unregistered Event type."
            );

            return;
        }

        if (
            !IsAllowed(
                descriptor.TypeName
            )
        ) {
            _output->println(
                "Operator dispatch is not permitted for this Event type."
            );

            Audit(
                LogLevel::Warning,
                "Denied operator Event dispatch."
            );

            return;
        }

        if (!descriptor.CanConstruct) {
            _output->println(
                "Event type is registered but cannot be runtime-constructed."
            );

            return;
        }

        Serializable::JsonArchive
            archive;

        const std::string jsonText(
            json
        );

        if (!archive.Load(jsonText)) {
            _output->println(
                "Invalid JSON object."
            );

            Audit(
                LogLevel::Warning,
                "Rejected malformed Event JSON."
            );

            return;
        }

        auto construction =
            _manager->
                CreateSerializableEvent(
                    descriptor.TypeID,
                    archive.GetNode(),
                    _config.
                        DeserializationOptions
                );

        if (!construction) {
            if (
                !construction.
                    TypeRegistered
            ) {
                _output->println(
                    "Event registration disappeared before construction."
                );
            } else if (
                !construction.
                    Constructible
            ) {
                _output->println(
                    "Event is not runtime-constructible."
                );
            }

            PrintConstructionIssues(
                construction.
                    Deserialization
            );

            Audit(
                LogLevel::Warning,
                "Serializable Event construction failed."
            );

            return;
        }

        _pendingEvent =
            std::move(
                construction.Event
            );

        _pendingTypeName =
            descriptor.TypeName;

        _pendingMethod = method;
        _pendingPriority =
            _config.DefaultPriority;
        _pendingDefaultDirection =
            descriptor.DefaultDirection;

        if (_config.RequireConfirmation) {
            RequestConfirmation();
        } else {
            DispatchPending();
        }
    }


    void BeginCompose(
        std::string_view arguments
    ) {
        const auto [
            typeName,
            remainder
        ] = SplitFirst(arguments);

        if (typeName.empty()) {
            _output->println(
                "Usage: event compose <stable-type-name> [queue|stack]"
            );

            return;
        }

        Event::SerializableEventDescriptor
            descriptor;

        if (
            !_manager->
                FindRegisteredSerializableEvent(
                    typeName,
                    descriptor
                )
        ) {
            _output->println(
                "Event type is not registered."
            );

            return;
        }

        if (
            !IsAllowed(
                descriptor.TypeName
            )
        ) {
            _output->println(
                "Operator dispatch is not permitted for this Event type."
            );

            return;
        }

        if (!descriptor.CanConstruct) {
            _output->println(
                "Event type cannot be runtime-constructed."
            );

            return;
        }

        Event::EventDispatchMethod method =
            Event::EventDispatchMethod::Queue;

        if (!remainder.empty()) {
            if (
                EqualsIgnoreCase(
                    remainder,
                    "stack"
                )
            ) {
                method =
                    Event::
                        EventDispatchMethod::
                            Stack;
            } else if (
                !EqualsIgnoreCase(
                    remainder,
                    "queue"
                )
            ) {
                _output->println(
                    "Compose method must be 'queue' or 'stack'."
                );

                return;
            }
        }

        if (
            method ==
                Event::
                    EventDispatchMethod::
                        Queue &&
            !_config.AllowQueue
        ) {
            _output->println(
                "Queue dispatch is disabled."
            );

            return;
        }

        if (
            method ==
                Event::
                    EventDispatchMethod::
                        Stack &&
            !_config.AllowStack
        ) {
            _output->println(
                "Stack dispatch is disabled."
            );

            return;
        }

        _pendingTypeName =
            descriptor.TypeName;

        _pendingMethod = method;
        _state =
            InteractionState::
                AwaitingJson;

        _output->print(
            "Enter one-line JSON object for "
        );

        _output->print(
            descriptor.TypeName.c_str()
        );

        _output->println(
            " (or 'cancel'):"
        );
    }


    bool HandleInteractiveLine(
        std::string_view line
    ) {
        std::lock_guard<std::mutex>
            lock(_mutex);

        line = Trim(line);

        if (
            _state ==
            InteractionState::None
        ) {
            return false;
        }

        if (
            EqualsIgnoreCase(
                line,
                "cancel"
            )
        ) {
            CancelPending(
                "Event composition cancelled."
            );

            return true;
        }

        if (
            _state ==
            InteractionState::
                AwaitingJson
        ) {
            const auto typeName =
                _pendingTypeName;

            const auto method =
                _pendingMethod;

            _pendingTypeName.clear();
            _state =
                InteractionState::None;

            ConstructAndDispatch(
                typeName,
                line,
                method
            );

            return true;
        }

        if (
            _state ==
            InteractionState::
                AwaitingConfirmation
        ) {
            if (
                EqualsIgnoreCase(
                    line,
                    "y"
                ) ||
                EqualsIgnoreCase(
                    line,
                    "yes"
                )
            ) {
                DispatchPending();

                return true;
            }

            CancelPending(
                "Event dispatch cancelled."
            );

            return true;
        }

        return false;
    }


    void HandleEventCommand(
        std::string_view arguments
    ) {
        const auto [
            subcommand,
            remainder
        ] = SplitFirst(arguments);

        if (
            subcommand.empty()
        ) {
            PrintUsage();
            return;
        }

        if (
            EqualsIgnoreCase(
                subcommand,
                "list"
            )
        ) {
            ListEvents();
            return;
        }

        if (
            EqualsIgnoreCase(
                subcommand,
                "describe"
            )
        ) {
            DescribeEvent(remainder);
            return;
        }

        if (
            EqualsIgnoreCase(
                subcommand,
                "cancel"
            )
        ) {
            std::lock_guard<std::mutex>
                lock(_mutex);

            CancelPending(
                "Pending Event operation cancelled."
            );

            return;
        }

        if (
            EqualsIgnoreCase(
                subcommand,
                "compose"
            )
        ) {
            std::lock_guard<std::mutex>
                lock(_mutex);

            BeginCompose(remainder);
            return;
        }

        if (
            EqualsIgnoreCase(
                subcommand,
                "queue"
            ) ||
            EqualsIgnoreCase(
                subcommand,
                "stack"
            ) ||
            EqualsIgnoreCase(
                subcommand,
                "dispatch"
            )
        ) {
            const auto [
                typeName,
                json
            ] = SplitFirst(remainder);

            Event::EventDispatchMethod
                method =
                    Event::
                        EventDispatchMethod::
                            Queue;

            if (
                EqualsIgnoreCase(
                    subcommand,
                    "stack"
                )
            ) {
                method =
                    Event::
                        EventDispatchMethod::
                            Stack;
            }

            if (
                method ==
                    Event::
                        EventDispatchMethod::
                            Queue &&
                !_config.AllowQueue
            ) {
                _output->println(
                    "Queue dispatch is disabled."
                );

                return;
            }

            if (
                method ==
                    Event::
                        EventDispatchMethod::
                            Stack &&
                !_config.AllowStack
            ) {
                _output->println(
                    "Stack dispatch is disabled."
                );

                return;
            }

            std::lock_guard<std::mutex>
                lock(_mutex);

            ConstructAndDispatch(
                typeName,
                json,
                method
            );

            return;
        }

        PrintUsage();
    }


public:
    bool Initialize(
        CommandConsole& commandConsole,
        const EventConsoleConfig& config = {},
        Event::EventTransportManager& manager = Event::EventTransportManager::GetInstance()
    ) {
        Shutdown();
        Console* console = commandConsole.GetConsole();
        Command::CommandRegistry* registry = commandConsole.GetRegistry();
        if (console == nullptr || registry == nullptr || console->GetOutput() == nullptr) return false;

        _console = console;
        _output = console->GetOutput();
        _manager = &manager;
        _config = config;
        _commandBacked = true;

        _eventCommandRegistration = registry->RegisterCommand("event");
        auto& event = registry->Command("event").Description("Serializable Event discovery, description, composition and dispatch");

        event.Command("list").Description("List runtime-registered Serializable Events").OnExecute([this](const Command::CommandContext&) { ListEvents(); return Command::CommandResult::Ok(); });
        auto& describe = event.Command("describe").Description("Describe a runtime-registered Serializable Event");
        describe.Parameter<std::string>("type").Description("Stable Event type name");
        describe.OnExecute([this](const Command::CommandContext& c) { DescribeEvent(c.Get<std::string>("type")); return Command::CommandResult::Ok(); });

        event.Command("cancel").Description("Cancel a pending Event composition/dispatch").OnExecute([this](const Command::CommandContext&) { std::lock_guard<std::mutex> lock(_mutex); CancelPending("Pending Event operation cancelled."); return Command::CommandResult::Ok(); });

        auto& compose = event.Command("compose").Description("Interactively compose a Serializable Event");
        compose.Parameter<std::string>("type").Description("Stable Event type name");
        compose.Parameter<std::string>("method").Optional().Default("queue").OneOf({"queue", "stack"});
        compose.OnExecute([this](const Command::CommandContext& c) { std::string args = c.Get<std::string>("type") + " " + c.Get<std::string>("method"); std::lock_guard<std::mutex> lock(_mutex); BeginCompose(args); return Command::CommandResult::Ok(); });

        auto registerDispatch = [this, &event](const char* name, Event::EventDispatchMethod method) {
            auto& node = event.Command(name).Description(std::string("Dispatch a Serializable Event via ") + (method == Event::EventDispatchMethod::Stack ? "Stack" : "Queue"));
            node.Parameter<std::string>("type").Description("Stable Event type name");
            node.Parameter<std::string>("json").Description("One-line JSON payload");
            node.OnExecute([this, method](const Command::CommandContext& c) {
                if ((method == Event::EventDispatchMethod::Queue && !_config.AllowQueue) || (method == Event::EventDispatchMethod::Stack && !_config.AllowStack)) return Command::CommandResult::Error("Requested dispatch method is disabled");
                std::lock_guard<std::mutex> lock(_mutex);
                ConstructAndDispatch(c.Get<std::string>("type"), c.Get<std::string>("json"), method);
                return Command::CommandResult::Ok();
            });
        };
        registerDispatch("queue", Event::EventDispatchMethod::Queue);
        registerDispatch("stack", Event::EventDispatchMethod::Stack);
        registerDispatch("dispatch", Event::EventDispatchMethod::Queue);

        _eventsCommandRegistration = registry->RegisterCommand("events");
        registry->Command("events").Description("List runtime-registered Serializable Events").OnExecute([this](const Command::CommandContext&) { ListEvents(); return Command::CommandResult::Ok(); });

        _interceptorID = console->RegisterLineInterceptor([this](std::string_view line) { return HandleInteractiveLine(line); });
        if (_interceptorID == 0) { Shutdown(); return false; }
        return true;
    }

    EventConsole() = default;

    EventConsole(
        const EventConsole&
    ) = delete;

    EventConsole& operator=(
        const EventConsole&
    ) = delete;


    ~EventConsole() {
        Shutdown();
    }


    bool Initialize(
        Console& console,
        const EventConsoleConfig&
            config = {},
        Event::
            EventTransportManager&
                manager =
                    Event::
                        EventTransportManager::
                            GetInstance()
    ) {
        if (
            console.GetOutput() ==
            nullptr
        ) {
            return false;
        }

        _console = &console;
        _output =
            console.GetOutput();

        _manager = &manager;
        _config = config;

        const bool eventRegistered =
            console.RegisterCommand(
                "event",
                "Serializable Event discovery, description, composition and dispatch",
                [this](
                    const ConsoleCommandContext&
                        context
                ) {
                    HandleEventCommand(
                        context.Arguments
                    );
                }
            );

        const bool eventsRegistered =
            console.RegisterCommand(
                "events",
                "List runtime-registered Serializable Events",
                [this](
                    const ConsoleCommandContext&
                ) {
                    ListEvents();
                }
            );

        if (
            !eventRegistered ||
            !eventsRegistered
        ) {
            if (eventRegistered) {
                console.UnregisterCommand(
                    "event"
                );
            }

            if (eventsRegistered) {
                console.UnregisterCommand(
                    "events"
                );
            }

            _console = nullptr;
            _output = nullptr;
            _manager = nullptr;

            return false;
        }

        _interceptorID =
            console.RegisterLineInterceptor(
                [this](
                    std::string_view line
                ) {
                    return
                        HandleInteractiveLine(
                            line
                        );
                }
            );

        if (_interceptorID == 0) {
            console.UnregisterCommand("event");
            console.UnregisterCommand("events");
            _console = nullptr;
            _output = nullptr;
            _manager = nullptr;
            return false;
        }

        return true;
    }


    void Shutdown() {
        if (_console != nullptr && !_commandBacked) {
            _console->
                UnregisterCommand(
                    "event"
                );

            _console->
                UnregisterCommand(
                    "events"
                );

            if (_interceptorID != 0) {
                _console->
                    UnregisterLineInterceptor(
                        _interceptorID
                    );
            }
        }

        {
            std::lock_guard<std::mutex>
                lock(_mutex);

            CancelPending();
        }

        _eventCommandRegistration.Reset();
        _eventsCommandRegistration.Reset();
        _commandBacked = false;
        _interceptorID = 0;
        _console = nullptr;
        _output = nullptr;
        _manager = nullptr;
        _auditSink = nullptr;
    }


    void SetAuditSink(
        ILoggerSink* sink
    ) noexcept {
        _auditSink = sink;
    }


    void SetAccessPolicy(
        EventConsoleAccessPolicy policy
    ) {
        std::lock_guard<std::mutex>
            lock(_mutex);

        _config.AccessPolicy =
            policy;
    }


    EventConsoleAccessPolicy
    GetAccessPolicy() const {
        std::lock_guard<std::mutex>
            lock(_mutex);

        return
            _config.AccessPolicy;
    }


    void AllowAllRegisteredEvents() {
        SetAccessPolicy(
            EventConsoleAccessPolicy::
                AllRegistered
        );
    }


    void RequireExplicitAllowList() {
        SetAccessPolicy(
            EventConsoleAccessPolicy::
                AllowListedOnly
        );
    }


    bool AllowEvent(
        std::string_view typeName
    ) {
        if (typeName.empty()) {
            return false;
        }

        std::lock_guard<std::mutex>
            lock(_mutex);

        _allowList.insert(
            std::string(typeName)
        );

        return true;
    }


    template<typename TEvent>
    bool AllowEvent() {
        constexpr auto name =
            Event::
                EventTransportTypeTraits<
                    TEvent
                >::Name;

        static_assert(
            !name.empty(),
            "AllowEvent<TEvent>() requires ESPRESSIO_EVENT_TRANSPORT_TYPE."
        );

        return AllowEvent(name);
    }


    bool DenyEvent(
        std::string_view typeName
    ) {
        if (typeName.empty()) {
            return false;
        }

        std::lock_guard<std::mutex>
            lock(_mutex);

        _denyList.insert(
            std::string(typeName)
        );

        return true;
    }


    template<typename TEvent>
    bool DenyEvent() {
        constexpr auto name =
            Event::
                EventTransportTypeTraits<
                    TEvent
                >::Name;

        static_assert(
            !name.empty(),
            "DenyEvent<TEvent>() requires ESPRESSIO_EVENT_TRANSPORT_TYPE."
        );

        return DenyEvent(name);
    }


    bool RemoveAllowEvent(
        std::string_view typeName
    ) {
        std::lock_guard<std::mutex>
            lock(_mutex);

        return
            _allowList.erase(
                std::string(typeName)
            ) > 0;
    }


    bool RemoveDenyEvent(
        std::string_view typeName
    ) {
        std::lock_guard<std::mutex>
            lock(_mutex);

        return
            _denyList.erase(
                std::string(typeName)
            ) > 0;
    }


    void ClearAccessLists() {
        std::lock_guard<std::mutex>
            lock(_mutex);

        _allowList.clear();
        _denyList.clear();
    }
};

}
