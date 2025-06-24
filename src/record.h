#ifndef RECORD_H
#define RECORD_H

#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <iostream>

enum RecordingMode {
    NORMAL_MODE,
    RECORDING_MODE,
    PLAYBACK_MODE
};

class MovementRecorder {
public:
    RecordingMode currentMode;
    std::vector<Matrix> recordedTransforms;  // Zapisane jointTransforms[7]
    std::vector<bool> recordedGrabStates;
    int playbackIndex;
    bool isRecording;
    bool isPlaying;
    bool isPlayingReverse;
    int frameCounter;
    
    MovementRecorder() {
        currentMode = NORMAL_MODE;
        playbackIndex = 0;
        isRecording = false;
        isPlaying = false;
        isPlayingReverse = false;
    }
    
    void StartRecording(Matrix initialTransforms[7]) {
        currentMode = RECORDING_MODE;
        isRecording = true;
        recordedTransforms.clear();
        recordedGrabStates.clear();
        std::cout << "RECORDING STARTED" << std::endl;
        
        // Zapisz pozycję startową (wszystkie 7 macierzy)
        for(int i = 0; i < 7; i++) {
            recordedTransforms.push_back(initialTransforms[i]);
        }
        recordedGrabStates.push_back(false);
    }
    
    void StopRecording() {
        currentMode = NORMAL_MODE;
        isRecording = false;
        std::cout << "RECORDING STOPPED - Saved " << recordedTransforms.size()/7 << " frames" << std::endl;
    }
    
    void StartPlayback() {
        if (recordedTransforms.empty()) {
            std::cout << "No recording to play!" << std::endl;
            return;
        }
        currentMode = PLAYBACK_MODE;
        isPlaying = true;
        isPlayingReverse = true;
        int totalFrames = recordedTransforms.size()/7;
        playbackIndex = totalFrames - 1;
        frameCounter = 0;  // Reset licznika
        std::cout << "PLAYBACK STARTED - " << recordedTransforms.size()/7 << " frames" << std::endl;
    }

    Matrix* GetCurrentPlaybackFrame() {
        int totalFrames = recordedTransforms.size()/7;
        
        // Sprawdź czy skończyliśmy odtwarzanie w tył (pierwsze)
        if (isPlayingReverse && playbackIndex < 0) {
            // Przełącz na odtwarzanie w przód
            isPlayingReverse = false;
            playbackIndex = 0;  // Zacznij od końca
            std::cout << "FORWARD PLAYBACK STARTED" << std::endl;
        }
        
        // Sprawdź czy skończyliśmy odtwarzanie w przód (drugie)
        if (!isPlayingReverse && playbackIndex >= totalFrames) {
            // Koniec całego odtwarzania
            currentMode = NORMAL_MODE;
            isPlaying = false;
            std::cout << "PLAYBACK FINISHED - Returned to start position" << std::endl;
            return nullptr;
        }
        
        // Zwróć aktualną ramkę
        return &recordedTransforms[playbackIndex * 7];
    }

    void UpdatePlayback() {
        if (currentMode == PLAYBACK_MODE && isPlaying) {
            frameCounter++;
            // Co każdą klatkę przejdź do następnej ramki (można spowolnić)
            if (frameCounter >= 1) {  // Zmień na 2 lub 3 aby spowolnić
                if (!isPlayingReverse) {
                    playbackIndex++;  // W przód
                } else {
                    playbackIndex--;  // W tył
                }
                frameCounter = 0;
            }
        }
    }
    
    void Update(Matrix currentTransforms[7], bool currentGrabState) {
        if (isRecording) {
            // Zapisuj aktualne transformacje (wszystkie 7)
            for(int i = 0; i < 7; i++) {
                recordedTransforms.push_back(currentTransforms[i]);
            }
            // Zapisuj stan grab
            recordedGrabStates.push_back(currentGrabState);
        }
    }

    Matrix* GetInitialTransforms() {
        if (recordedTransforms.empty()) return nullptr;
        // Zwróć pierwsze 7 macierzy (pozycja startowa)
        return &recordedTransforms[0];
    }
    
    bool HasRecording() {
        return !recordedTransforms.empty();
    }

    bool GetCurrentGrabState() {  
        if (recordedGrabStates.empty() || playbackIndex >= recordedGrabStates.size()) {
            return false;
        }
        return recordedGrabStates[playbackIndex];
    }
};

#endif