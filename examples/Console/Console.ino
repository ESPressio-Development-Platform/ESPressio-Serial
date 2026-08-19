#include <Arduino.h>
#include <ESPressio_Console.hpp>

ESPressio::Serial::Console console;

void setup() {
    ::Serial.begin(115200);

    ESPressio::Serial::ConsoleConfig config;
    config.ShowPrompt = true;
    config.Prompt = "espressio> ";

    console.Initialize(
        ::Serial,
        ::Serial,
        config
    );

    console.RegisterCommand(
        "hello",
        "Print a greeting",
        [](const auto& context) {
            ::Serial.print("Hello");

            if (!context.Arguments.empty()) {
                ::Serial.print(", ");
                ::Serial.write(
                    reinterpret_cast<const uint8_t*>(
                        context.Arguments.data()
                    ),
                    context.Arguments.size()
                );
            }

            ::Serial.println("!");
        }
    );
}

void loop() {
    console.Poll();
    delay(1);
}
