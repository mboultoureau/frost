# Frost Engine

Frost is a game engine created by the Uno team ([Elias Del Pozo](https://depot.dinf.usherbrooke.ca/dele5830), [Mathis Boultoureau](https://depot.dinf.usherbrooke.ca/boum6233), [Simon Le Floch](https://depot.dinf.usherbrooke.ca/lefs1934) and [Thomas Vanwalleghem](https://depot.dinf.usherbrooke.ca/vant2523)) for DDJV courses.

## Installation

To install and run the project, you must be running Windows and have Visual Studio 2022 and CMake installed. Then run the `Setup.bat` script to generate the project.

## Resources

- [Game Programming Patterns](https://gameprogrammingpatterns.com/)
- [Real-Time Rendering](https://www.realtimerendering.com/)
- [h-deb - Patrice Roy](https://h-deb.ca/)
- [Game Engine Series - The Cherno](https://youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT)

## Docs

### Dependencies

Here are the dependencies present in the engine:
- [assimp](https://github.com/assimp/assimp): importing 3D models
- [imgui](https://github.com/ocornut/imgui): debug interface
- [JoltPhysics](https://github.com/jrouwe/JoltPhysics): physics engine

The various dependencies are located in Frost/vendor for dependencies specific to the engine and Lab/vendor for dependencies specific to the Lab using git submodule.

To update dependencies to their latest versions, you can use: `git submodule update --remote --merge`.


### Entity Component System

### Event System

Sources :

- [Event Queue - Game Programming Patterns](https://gameprogrammingpatterns.com/event-queue.html)
- [Event System - Denys Kryvytskyi](https://denyskryvytskyi.github.io/event-system)

### Utils

Voici les différentes classes utilitaires présentes dans le projet :

- NoCopy : peut-être utiliser comme héritage pour bloquer le constructeur de copie et l'opérateur d'affectation public.
- UUID : permet de générer un identifiant unique. L'UUID est pour le moment sur 64 bits et repose pour le moment sur un générateur aléatoire non cryptographique. Il existe un risque de collisions (qui est environ de 50% après 2**32 générations).

Sources :

- [Code de grande personne – bloquer la copie - Patrice Roy](https://h-deb.ca/Sujets/Divers--cplusplus/Incopiable.html)
- [Génération de nombres pseudoaléatoires avec C++ - Patrice Roy](https://h-deb.ca/Sujets/Divers--cplusplus/prng.html)

