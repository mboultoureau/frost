#include "HUD/Timer.h"
#include "GameState/GameState.h"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <variant>
using namespace Frost;
using namespace Frost::Component;
namespace GameLogic
{
    void Timer::OnCreate()
    {
        auto scene = GetGameObject().GetScene();

        // Création de l'objet UI pour le texte
        _timerTextObj = scene->CreateGameObject("Race Timer Text");

        // Configuration initiale
        // On commence avec un texte vide pendant les 5 secondes d'attente
        auto font = AssetManager::LoadAsset<Font>(fontPath);
        std::string initText = "";

        auto& textElement = _timerTextObj.AddComponent<UIElement>(
            UIElement{ .content = UIText{ .text = initText,
                                          .font = font,
                                          .fontFilepath = fontPath,
                                          .fontSize = 5.0f } }); // Grande taille pour le countdown

        // Viewport centré pour le compte à rebours
        Viewport viewportCenter;
        viewportCenter.x = 0.45f;
        viewportCenter.y = 0.4f;
        viewportCenter.width = 0.5f;
        viewportCenter.height = 0.5f;

        textElement.viewport = viewportCenter;
        textElement.priority = 10;
        textElement.color = { 1.0f, 1.0f, 1.0f, 1.0f };

        _waitTime = 5.0f;      // Temps d'attente avant le décompte
        _countdownTime = 3.0f; // Durée du décompte (3, 2, 1)
        _raceTime = 0.0f;

        _isWaiting = true;
        _isRaceStarted = false;
        _isFinished = false;
    }

    void Timer::OnDestroy()
    {
        auto scene = GetGameObject().GetScene();
        if (scene)
        {
            scene->DestroyGameObject(_timerTextObj);
        }
    }

    void Timer::OnUpdate(float deltaTime)
    {
        // Si le GameState indique que c'est fini, on arrête la mise à jour visuelle
        if (GameState::Get().Finished())
        {
            _isFinished = true;
            return;
        }

        // 1. Phase d'attente (5 secondes)
        if (_isWaiting)
        {
            _waitTime -= deltaTime;
            if (_waitTime <= 0.0f)
            {
                _isWaiting = false;
                // On force l'affichage du "3" immédiatement à la fin de l'attente
                SetText("3");
            }
            return;
        }

        // 2. Phase de décompte (3, 2, 1)
        if (!_isRaceStarted)
        {
            UpdateCountdown(deltaTime);
        }
        // 3. Phase de course (Chrono)
        else
        {
            UpdateRaceTimer(deltaTime);
        }
    }

    void Timer::UpdateCountdown(float deltaTime)
    {
        _countdownTime -= deltaTime;

        int displayTime = static_cast<int>(std::ceil(_countdownTime));

        // Mise à jour du texte (3, 2, 1)
        if (displayTime > 0)
        {
            SetText(std::to_string(displayTime));
        }
        else
        {
            // Le compte à rebours est fini : Start !
            StartRace();
        }
    }

    void Timer::StartRace()
    {
        _isRaceStarted = true;
        _raceTime = 0.0f;

        // On change le style pour le timer de course (plus petit, en haut à gauche)
        auto& uiElement = _timerTextObj.GetComponent<UIElement>();

        if (std::holds_alternative<UIText>(uiElement.content))
        {
            std::get<UIText>(uiElement.content).fontSize = 1.0f; // Taille normale
        }

        // Viewport en coin (Top-Left approx)
        Viewport viewportCorner;
        viewportCorner.x = 0.05f;
        viewportCorner.y = 0.05f;
        viewportCorner.width = 0.5f;
        viewportCorner.height = 0.5f;

        uiElement.viewport = viewportCorner;

        // Logique optionnelle : Débloquer le joueur ici si nécessaire
    }

    void Timer::UpdateRaceTimer(float deltaTime)
    {
        _raceTime += deltaTime;

        // Calcul du temps
        int minutes = static_cast<int>(_raceTime) / 60;
        int seconds = static_cast<int>(_raceTime) % 60;
        int milliseconds = static_cast<int>((_raceTime - static_cast<int>(_raceTime)) * 1000);

        // Formatage MM:SS:ms
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << minutes << ":" << std::setfill('0') << std::setw(2) << seconds << ":"
           << std::setfill('0') << std::setw(3) << milliseconds;

        SetText(ss.str());
    }

    void Timer::SetText(const std::string& text)
    {
        if (!_timerTextObj.IsValid())
            return;

        auto& uiElement = _timerTextObj.GetComponent<UIElement>();
        if (std::holds_alternative<UIText>(uiElement.content))
        {
            std::get<UIText>(uiElement.content).text = text;
        }
    }
} // namespace GameLogic