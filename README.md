# Frost Engine

Frost is a game engine created by the Uno team ([Elias Del Pozo](https://depot.dinf.usherbrooke.ca/dele5830), [Mathis Boultoureau](https://depot.dinf.usherbrooke.ca/boum6233), [Simon Le Floch](https://depot.dinf.usherbrooke.ca/lefs1934) and [Thomas Vanwalleghem](https://depot.dinf.usherbrooke.ca/vant2523)) for DDJV courses.

## Installation

To install and run the project, you must be running Windows and have Visual Studio 2022 and CMake installed. Then run the `Setup.bat` script to generate the project.

## Features

- [x] ECS
- [x] Rendering (DirectX 11)
- [x] Physics (Jolt)
- [ ] Input (mouse, keyboard, gamepad)
- [x] Debugging (logger, debugging interface)
- [x] Easily extensible

## Resources

- [Game Programming Patterns](https://gameprogrammingpatterns.com/)
- [Real-Time Rendering](https://www.realtimerendering.com/)
- [Game Engine Architecture](https://www.gameenginebook.com/)
- [h-deb - Patrice Roy](https://h-deb.ca/)
- [Game Engine Series - The Cherno](https://youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT)

## Dependencies

Here are the dependencies present in the engine:
- [assimp](https://github.com/assimp/assimp): importing 3D models
- [imgui](https://github.com/ocornut/imgui): debug interface
- [JoltPhysics](https://github.com/jrouwe/JoltPhysics): physics engine
- [spdlog](https://github.com/gabime/spdlog): logging

The various dependencies are located in Frost/vendor for dependencies specific to the engine and Lab/vendor for dependencies specific to the Lab using git submodule.

To update dependencies to their latest versions, you can use: `git submodule update --remote --merge`.

## Docs

### Logger and Asserts

The engine uses spdlog for logging. There are two loggers available: one for the engine and one for the game. The loggers support different log levels: trace, info, warn, error and critical.

```cpp
// Engine logger
FT_ENGINE_TRACE("Initializing renderer subsystem...");
FT_ENGINE_INFO("Mouse button {} pressed", mouseButton);
FT_ENGINE_WARN("Frame time is high: {} ms", 18.3f);
FT_ENGINE_ERROR("Failed to load shader '{}'", "water_reflection");
FT_ENGINE_CRITICAL("Renderer crashed! Error code: {}", -1);

// Game logger
FT_TRACE("Player {} entered the world", playerName);
FT_INFO("Player health: {}", health);
FT_WARN("Player {} is lagging (connected: {})", playerName, isConnected);
FT_ERROR("Cannot spawn entity ID {}", 42);
FT_CRITICAL("Fatal error: out of memory!");

// Asserts
FT_ENGINE_ASSERT(_buttonStates[i] != ButtonState::Pressed);
FT_ASSERT(_buttonStates[i] != ButtonState::Pressed, "Button state should be Pressed after pressing the button");
```

### Entity Component System

### Event System

Sources :

- [Event Queue - Game Programming Patterns](https://gameprogrammingpatterns.com/event-queue.html)
- [Event System - Denys Kryvytskyi](https://denyskryvytskyi.github.io/event-system)

### Utils

Here are the different utility classes included in the project:
- NoCopy: can be used as inheritance to block the copy constructor and public assignment operator.
- UUID: allows you to generate a unique identifier.
- Maths utilities: function to manage angle conversion or approximation between two floats.

Sources :

- [Code de grande personne – bloquer la copie - Patrice Roy](https://h-deb.ca/Sujets/Divers--cplusplus/Incopiable.html)
- [Implémenter des conversions de référentiels - Patrice Roy](https://h-deb.ca/Sujets/Divers--cplusplus/Implementer-changement-referentiel.html)
