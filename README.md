# SwiftBot : Portal Rush

## Jeu

__But du jeu :__ le but du jeu est de faire 3 en tours le plus rapidement possible.

_Mécaniques :__ 
- Changement de véhicule : 
    - moto -> rapide, boost dérapage, roule sur les murs, 
	- bateau -> flotte sur l'eau, ne roule pas
	- planneur -> plane dans les air, rapide en descente, ne roule pas
- Portail -> les portails t'emmène plus vite vers la sortie … ou non !


Contrôle : Player (qwerty)
- w -> vers l'avant, s -> vers l'arrière, a -> vers la gauche, d -> vers la droite, 
- shift : brake, espace -> brake + boost (dans les virages), q -> véhicule vers la gauche,
- e -> véhicule vers la droite, clic molette -> 1ère personne
  
Contrôle : Autres (qwerty)
- echap -> pause, R -> reload, C -> souris camera rotation, F1 -> menu debug

## Auteurs

Equipe : UNO
Nom : Boultoureau, Del Pozo, Le Floch, Vanwallalleghem

Moteur : Frost (moteur custom)

## Installation

- Décompressez le zip
- Exécutez Setup.bat
- Ouvrir la solution Frost.sln
- S'assurez que Game est sélectionné en mode Debug
- Lancez le jeu

## Objets

Liste des Objets : Un terrain, 3 véhicules (moto, bateau, planneur), Checkpoint(ligne d'arrivée), Herbe, Block d'eau, Billboard (flèche indicatrice)

HUD / Menu Pause : Nous avons des HUD images dynamique (compteur vitesse) et statique(image victory), du texte dynamique  (compteur vitesse) et statique (CountDown), puis des boutons cliquables (Play). Le "Menu Pause" en est un composée tout comme le "Menu Start" et "Menu Victory"

SkyBox : Nous pouvons ajouter 6 images différentes pour la boîte dans le ciel

Light : Nous gérons 3 types de light (directionnel, sphere/point, phare)
	L'éclairage est de type "Blinn-Phong".

Texture : Nous avons X texture sur notre map. 

Post-Process : 
	-Chromatic Aberation : passage dans les portails
	-Screen Shake : collisions
	-Radial Blur : selon la vitesse du joueur
	-Fog : vision bleu si on est dans l'eau 
	-Toon Shading : composée de "Toon" et de "Outline" qui rend un coté cartoon au jeu 

Material : 
	-Grass : de l'herbe qui bouge au vent 
	-Water : les vagues sur l'eau 
	-Billboard : Image avec transparence qui suit la direction de la camera

Organisateur de Scène :  
	- Culling : les objets du terrain est affichée seulement dans un cône directionnel de la vue du joueur
	- ECS (Entity Component System): Il assure une architecture modulaire en séparant les données (Components) de la logique (Systems). Optimisation pour toutes les mises à jour des objets du jeu.

Rendering : Le moteur est en mode Deferred Rendering

## Bibliothèques externes  
- assimp : chargement des modèles 3D
- entt : système ECS
- imgui : menu de debug
- JoltPhysics : moteur physique
- spldlog : affichage et stockage des logs
- stb : chargements des images et polices
- yaml-cpp : stockage en yaml des scènes et prefabs (uniquement dans l'éditeur)


Lance le jeu et amuse toi bien !!!!