#ifndef ROBOT_H
#define ROBOT_H

#include "raylib.h"
#include "raymath.h"
#include "rlights.h"
#include "object.h"
#include <iostream>


//klasa odpowiedzialna za generowanie, transformacje, rysowanie i unloadowanie poszczególnych części
class RobotPart {
public:
    Model model; //model częsci
    Vector3 position; //pozycja częsci

    RobotPart() {}
    
    //Konstruktor - tworzy model, daje materiał do shadera i pozycje obiektu
    RobotPart(const char* filePath, Shader shader, Vector3 pos) {
        model = LoadModel(filePath);
        model.materials[0].shader = shader;
        position = pos;
    }
    //funkcja do obracania
    void SetTransform(Matrix transform) { 
        model.transform = transform;
    }
    //funkcja do rysowania
    void Draw(Color tint) {
        DrawModel(model, {0.0f, 0.0f, 0.0f}, 1.0f, tint);
    }
    //funkcja do unloadowania
    void Unload() {
        UnloadModel(model);
    }
    //funkcja do rysowania osi
    void DrawAxes(float axisLength = 10.0f) const {
        Vector3 worldPos = Vector3Transform(Vector3Zero(), model.transform);
        Vector3 axisX = Vector3Transform(Vector3{1, 0, 0}, model.transform);
        Vector3 axisY = Vector3Transform(Vector3{0, 1, 0}, model.transform);
        Vector3 axisZ = Vector3Transform(Vector3{0, 0, 1}, model.transform);

        axisX = Vector3Scale(Vector3Normalize(Vector3Subtract(axisX, worldPos)), axisLength);
        axisY = Vector3Scale(Vector3Normalize(Vector3Subtract(axisY, worldPos)), axisLength);
        axisZ = Vector3Scale(Vector3Normalize(Vector3Subtract(axisZ, worldPos)), axisLength);

        DrawLine3D(worldPos, Vector3Add(worldPos, axisX), RED);
        DrawLine3D(worldPos, Vector3Add(worldPos, axisY), GREEN);
        DrawLine3D(worldPos, Vector3Add(worldPos, axisZ), BLUE);
    }
};

//reprezentacja całego robota
class Robot {
public:

    bool clampsOpen = true;
    
    //definiowanie części robota o konkretnych atrybutach z klasy RobotPart
    RobotPart waist;
    RobotPart shoulder;
    RobotPart arm;
    RobotPart base;
    RobotPart wrist_A;
    RobotPart wrist_B; 
    RobotPart wrist_C;
    RobotPart clamps_A;
    RobotPart clamps_B;
    RobotPart magnetic_wrist; // element chwytaka, "magnes"
    

    float pitch = 0.0f;     // Y-Axis (obrót)
    float roll = 0.0f;      // X-Axis (podnoszenie ramienia)
    float rollArm = 0.0f;   // X-Axis (zginanie przedramienia)
    float wrist_A_Rotation = 0.0f; 
    float wrist_B_Rotation = 0.0f;
    float wrist_C_Rotation = 0.0f; 

    //Pozycje startowe konkretnych elementów        // to jest useless obecnie
    Vector3 basepos = {0.0f, 0.0f, 0.0f}; //pozycja bazy robota
    Vector3 waistPos = { 0.0f, 0.0f, 0.0f };
    Vector3 shoulderPos = { 0.0f, 0.0f, 0.0f };
    Vector3 armPos = {0.0f, 0.0f, 0.0f};
    Vector3 wrist_A_Pos = {0.0f, 0.0f, 0.0f }; 
    Vector3 wrist_B_Pos = {0.0f, 0.0f, 0.0f };
    Vector3 wrist_C_Pos = {0.0f, 0.0f, 0.0f }; 

    struct DHParams {
        float theta, delta, lambda, alpha;
    };
    DHParams dhParams[7];  // parametry dla 6 przegubów
    Matrix jointTransforms[7];  // wynikowe macierze

    // Funkcja generująca macierz D-H
    Matrix DHMatrix(float theta, float delta, float lambda, float alpha) {
        float ct = cosf(theta), st = sinf(theta);
        float ca = cosf(alpha), sa = sinf(alpha);
            // ZAOKRĄGLANIE BARDZO MAŁYCH WARTOŚCI DO ZERA
        if (fabsf(ct) < 1e-6f) ct = 0.0f;
        if (fabsf(st) < 1e-6f) st = 0.0f;
        if (fabsf(ca) < 1e-6f) ca = 0.0f;
        if (fabsf(sa) < 1e-6f) sa = 0.0f;

        return Matrix{
            ct, -st*ca,  st*sa, lambda*ct,
            st,  ct*ca, -ct*sa, lambda*st,
            0,      sa,     ca,    delta,
            0,       0,      0,    1
        };
    }

    //konstrutor robota - tworzy siatki elementów, a potem całe modele
    Robot(Shader shader) {
        base = RobotPart("src/Pieza1_new.obj", shader, basepos);
        waist = RobotPart("src/Pieza2_new.obj", shader, waistPos); 
        shoulder = RobotPart("src/Pieza3_new.obj", shader, shoulderPos);
        arm = RobotPart("src/Pieza4_new.obj", shader, armPos);
        wrist_A = RobotPart("src/Pieza5.obj", shader, wrist_A_Pos); // pierwszy element chwytaka
        wrist_B = RobotPart("src/Pieza6.obj", shader, wrist_B_Pos); // drugi element chwytaka, "mały krzyżyk"
        wrist_C = RobotPart("src/Pieza8.obj", shader, wrist_C_Pos); // trzeci element chwytaka, "przyssawka"
        clamps_A = RobotPart("src/Pieza9.obj", shader, basepos);
        clamps_B = RobotPart("src/Pieza9.obj", shader, basepos);
        magnetic_wrist = RobotPart("src/Magnetic.obj", shader, basepos); // element chwytaka, "magnes"
    }

    void Update() {
        
        // Układ zerowy
        dhParams[0].theta = 0.0f;  
        dhParams[0].delta = 0.0f;                              
        dhParams[0].lambda = 0.0f; 
        dhParams[0].alpha = DEG2RAD*-90.0f;
        // Przegub 1: Biodro
        dhParams[1].theta = DEG2RAD * pitch;    // A/D
        dhParams[1].delta = 0.0f;                              
        dhParams[1].lambda = 0.0f; 
        dhParams[1].alpha = DEG2RAD*-90.0f;
        // Przegub 2: Ramię
        dhParams[2].theta = DEG2RAD * roll;     // W/S
        dhParams[2].delta = 6.7f;   // odsunięcie ramienia od biodra
        dhParams[2].lambda = 17.0f; // odległość układu ramienia od przedramienia                 
        dhParams[2].alpha  = 0.0f;
        // Przegub 3: Przedramię
        dhParams[3].theta = DEG2RAD * rollArm; 
        dhParams[3].delta = -1.5f;  // wraz z armToWristCorrection poprawia pozycję nadgarstka względem przedramienia, wcześniej delta 0.0f
        dhParams[3].lambda = 0.0f;
        dhParams[3].alpha = DEG2RAD*-90.0f;
        // Przegub 4: Nadgarstek A
        dhParams[4].theta = DEG2RAD * wrist_A_Rotation; 
        dhParams[4].delta = 14.7f + 2.36f; // +2.36 dodaje odpowiednią różnicę między układami nadgastków A i B, nadg. A cofany, patrz wrisAtoBCorrection// odległość nadgarstka A od końca przedramienia, tu zgodnie z obliczeniami wyszło 0 (jakby coś się chrzaniło)
        dhParams[4].lambda = 0.0f;
        dhParams[4].alpha = DEG2RAD*90.0f; 
        // Przegub 5: Nadgarstek B
        dhParams[5].theta = DEG2RAD * wrist_B_Rotation; // I K
        dhParams[5].delta = 0.0f; 
        dhParams[5].lambda = 0.0f;
        dhParams[5].alpha = DEG2RAD*-90.0f; 
        // Przegub 6: Nadgarstek C
        dhParams[6].theta = DEG2RAD * wrist_C_Rotation; // J L
        dhParams[6].delta = 2.4f;   // oddalenie ostatniej części nadgarstka od przedostatniej 
        dhParams[6].lambda = 0.0f;
        dhParams[6].alpha = DEG2RAD*90.0f; 

         // POPRAWNE ŁAŃCUCHOWE MNOŻENIE
        Matrix T_01 = DHMatrix(dhParams[0].theta, dhParams[0].delta, 
                                        dhParams[0].lambda, dhParams[0].alpha);
        Matrix T_12 = DHMatrix(dhParams[1].theta, dhParams[1].delta, 
                                        dhParams[1].lambda, dhParams[1].alpha);
        Matrix T_23 = DHMatrix(dhParams[2].theta, dhParams[2].delta, 
                                        dhParams[2].lambda, dhParams[2].alpha);
        Matrix T_34 = DHMatrix(dhParams[3].theta, dhParams[3].delta, 
                                        dhParams[3].lambda, dhParams[3].alpha);
        Matrix T_45 = DHMatrix(dhParams[4].theta, dhParams[4].delta, 
                                        dhParams[4].lambda, dhParams[4].alpha);
        Matrix T_56 = DHMatrix(dhParams[5].theta, dhParams[5].delta, 
                                        dhParams[5].lambda, dhParams[5].alpha);
        Matrix T_67 = DHMatrix(dhParams[6].theta, dhParams[6].delta, 
                                        dhParams[6].lambda, dhParams[6].alpha);
    
        jointTransforms[0] = T_01; // T_01       
        jointTransforms[1] = MatrixMultiply(T_12, jointTransforms[0]); // T_02 
        jointTransforms[2] = MatrixMultiply(T_23, jointTransforms[1]); // T_03
        jointTransforms[3] = MatrixMultiply(T_34, jointTransforms[2]); // T_04
        jointTransforms[4] = MatrixMultiply(T_45, jointTransforms[3]); // T_05 
        jointTransforms[5] = MatrixMultiply(T_56, jointTransforms[4]); // T_06  
        jointTransforms[6] = MatrixMultiply(T_67, jointTransforms[5]); // T_07                     

        base.SetTransform(MatrixMultiply(MatrixRotateX(DEG2RAD * -90.0f), MatrixTranslate(0.0f, -24.0f, 0.0f)));    // -24 to odległość poniżej zera aby baza była pod biodrem
        waist.SetTransform(MatrixMultiply(MatrixRotateX(DEG2RAD *90.0f), jointTransforms[1]));
        shoulder.SetTransform(MatrixMultiply(MatrixTranslate(-17.0f, 0.0f, 0.0f), jointTransforms[2])); 
        Matrix armToWristCorrection = MatrixMultiply(MatrixRotateX(DEG2RAD *90.0f), MatrixTranslate(0.0f, -1.5f, 0.0f));    //Translate wraz z dhParam[3].delta poprawia pozycję nadgarstka względem przedramienia, wcześniej 0,0,0
        arm.SetTransform(MatrixMultiply(armToWristCorrection, jointTransforms[3]));
        Matrix wristAtoBCorrection = MatrixMultiply(MatrixRotateX(DEG2RAD *90.0f), MatrixTranslate(0.0f, -2.36f, 0.0f));    
        wrist_A.SetTransform(MatrixMultiply(wristAtoBCorrection, jointTransforms[4])); 
        // TO BYŁO przed dodaniem wristAtoBCorrection // wrist_A.SetTransform(MatrixMultiply(MatrixRotateX(DEG2RAD *90.0f), jointTransforms[4])); 
        Matrix wristBCorrection = MatrixMultiply(MatrixRotateX(DEG2RAD *-90.0f), MatrixRotateZ(DEG2RAD *90.0f)); 
        wrist_B.SetTransform(MatrixMultiply(wristBCorrection, jointTransforms[5])); 
        // TO BYŁO PRZED wristBCorr.. wtedy alfa [5] byla 180 // wrist_B.SetTransform(MatrixMultiply(MatrixRotateY(DEG2RAD *90.0f), jointTransforms[5]));
        wrist_C.SetTransform(jointTransforms[6]);
        magnetic_wrist.SetTransform(jointTransforms[6]); 
        
        if (clampsOpen) {
            // Pozycja otwarta
            clamps_A.SetTransform(MatrixMultiply(MatrixTranslate(0.0f, 2.0f, 2.5f), jointTransforms[6])); 
            clamps_B.SetTransform(MatrixMultiply(MatrixTranslate(0.0f, 2.0f, -2.5f), jointTransforms[6])); 
        } else {
            // Pozycja zamknięta
            clamps_A.SetTransform(MatrixMultiply(MatrixTranslate(0.0f, 2.0f, 1.8f), jointTransforms[6])); 
            clamps_B.SetTransform(MatrixMultiply(MatrixTranslate(0.0f, 2.0f, -1.8f), jointTransforms[6])); 
        }

    }

    void SetFromTransforms(Matrix* transforms) {
        // Skopiuj linie 190-201 ale użyj przekazanych transforms zamiast jointTransforms
        base.SetTransform(MatrixMultiply(MatrixRotateX(DEG2RAD * -90.0f), MatrixTranslate(0.0f, -24.0f, 0.0f)));
        waist.SetTransform(MatrixMultiply(MatrixRotateX(DEG2RAD *90.0f), transforms[1]));
        shoulder.SetTransform(MatrixMultiply(MatrixTranslate(-17.0f, 0.0f, 0.0f), transforms[2])); 
        Matrix armToWristCorrection = MatrixMultiply(MatrixRotateX(DEG2RAD *90.0f), MatrixTranslate(0.0f, -1.5f, 0.0f));
        arm.SetTransform(MatrixMultiply(armToWristCorrection, transforms[3]));
        Matrix wristAtoBCorrection = MatrixMultiply(MatrixRotateX(DEG2RAD *90.0f), MatrixTranslate(0.0f, -2.36f, 0.0f));
        wrist_A.SetTransform(MatrixMultiply(wristAtoBCorrection, transforms[4])); 
        Matrix wristBCorrection = MatrixMultiply(MatrixRotateX(DEG2RAD *-90.0f), MatrixRotateZ(DEG2RAD *90.0f)); 
        wrist_B.SetTransform(MatrixMultiply(wristBCorrection, transforms[5])); 
        wrist_C.SetTransform(transforms[6]);

        if (clampsOpen) {
            clamps_A.SetTransform(MatrixMultiply(MatrixTranslate(0.0f, 2.0f, 2.5f), transforms[6])); 
            clamps_B.SetTransform(MatrixMultiply(MatrixTranslate(0.0f, 2.0f, -2.5f), transforms[6])); 
        } else {
            clamps_A.SetTransform(MatrixMultiply(MatrixTranslate(0.0f, 2.0f, 1.8f), transforms[6])); 
            clamps_B.SetTransform(MatrixMultiply(MatrixTranslate(0.0f, 2.0f, -1.8f), transforms[6])); 
        }
    }

    // Zmiana stanu zacisków
    void ToggleClamps() {
        clampsOpen = !clampsOpen;
    }

    //rysowanie elementów
    void Draw(bool gripperType) {
        base.Draw(LIGHTGRAY);
        waist.Draw(LIGHTGRAY);
        //waist.DrawAxes(30.0f);
        shoulder.Draw(LIGHTGRAY);
        //shoulder.DrawAxes(30.0f);
        arm.Draw(LIGHTGRAY);
        //arm.DrawAxes(30.0f);
        wrist_A.Draw(LIGHTGRAY); 
        //wrist_A.DrawAxes(30.0f);
        wrist_B.Draw(LIGHTGRAY);
        //wrist_B.DrawAxes(30.0f);
        if(gripperType) {
            wrist_C.Draw(LIGHTGRAY); 
            clamps_A.Draw(LIGHTGRAY); 
            clamps_B.Draw(LIGHTGRAY);
        } else {
            magnetic_wrist.Draw(LIGHTGRAY); // rysowanie elementu chwytaka, "magnes"
        }
        
    }
    //unloadowanie elementów
    void Unload() {
        waist.Unload();
        shoulder.Unload();
        arm.Unload();
        base.Unload();
        wrist_A.Unload();
        wrist_B.Unload();
        wrist_C.Unload();
        clamps_A.Unload();
        clamps_B.Unload();
        magnetic_wrist.Unload();
    }
        
    bool IsAboveGround(Matrix transform, bool grab, float minY = -23.0f) {
        transform = (MatrixMultiply(MatrixTranslate(0.0f, 3.1f, 0.0f), transform));
        Vector3 worldPos = Vector3Transform(Vector3Zero(), transform);
        return worldPos.y >= minY;
    }
    //inputy do sterowania
    void HandleInput(bool grab) {
        if (IsKeyDown(KEY_A)){
            if(pitch < 360.0f){
                pitch += 1.0f;}
            } 
        if (IsKeyDown(KEY_D)){
            if(pitch > -180.0f){
                pitch -= 1.0f;}
            }
        if (IsKeyDown(KEY_W)) {
            if (roll > -135.0f) { // ograniczenie do -135 stopni
                roll -= 1.0f;
            }
            Update();
            if (!IsAboveGround(jointTransforms[6], grab)) {
                roll += 1.0f;
                Update();
            }
        }
        if (IsKeyDown(KEY_S)) {
            if(roll < 90.0f) { // ograniczenie do 90 stopni
                roll += 1.0f;
            }
            Update();
            if (!IsAboveGround(jointTransforms[6], grab)) {
                roll -= 1.0f;
                Update();
            }
        }
        if (IsKeyDown(KEY_UP)) {
            if(rollArm > -180.0f) { // ograniczenie do -180 stopni
                rollArm -= 1.0f;
            }
            Update();
            if (!IsAboveGround(jointTransforms[6], grab)) {
                rollArm += 1.0f;
                Update();
            }
        }
        if (IsKeyDown(KEY_DOWN)) {
            if(rollArm < 45.0f) { // ograniczenie do 45 stopni
                rollArm += 1.0f;
            }
            Update();
            if (!IsAboveGround(jointTransforms[6], grab)) {
                rollArm -= 1.0f;
                Update();
            }
        }
        if (IsKeyDown(KEY_RIGHT)){
            if (wrist_A_Rotation > -90.0f) { // ograniczenie do -90 stopni
                wrist_A_Rotation -= 1.0f;
            }
            Update();
            if (!IsAboveGround(jointTransforms[6], grab)) {
                wrist_A_Rotation += 1.0f;
                Update();
            }
        }
        if (IsKeyDown(KEY_LEFT)){
            if (wrist_A_Rotation < 90.0f) { // ograniczenie do 90 stopni
                wrist_A_Rotation += 1.0f;
            }
            Update();
            if (!IsAboveGround(jointTransforms[6], grab)) {
                wrist_A_Rotation -= 1.0f;
                Update();
            }
        }
        if (IsKeyDown(KEY_I)){
            if( wrist_B_Rotation > -90.0f) { // ograniczenie do -90 stopni
                wrist_B_Rotation -= 1.0f;
            }
            Update();
            if (!IsAboveGround(jointTransforms[6], grab)) {
                wrist_B_Rotation += 1.0f;
                Update();
            }
        }
        if (IsKeyDown(KEY_K)){
            if( wrist_B_Rotation < 90.0f) { // ograniczenie do 90 stopni
                wrist_B_Rotation += 1.0f;
            }
            Update();
            if (!IsAboveGround(jointTransforms[6], grab)) {
                wrist_B_Rotation -= 1.0f;
                Update();
            }
        } 
        if (IsKeyDown(KEY_J)){
            if(wrist_C_Rotation < 90.0f){
                wrist_C_Rotation += 1.0f; // ograniczenie do 90 stopni
            }
            Update();
            if (!IsAboveGround(jointTransforms[6], grab)) {
                wrist_C_Rotation -= 1.0f;
                Update();
            } 
        }
        if (IsKeyDown(KEY_L)){
            if(wrist_C_Rotation > -90.0f){
                wrist_C_Rotation -= 1.0f; // ograniczenie do -90 stopni
            }
            Update();
            if (!IsAboveGround(jointTransforms[6], grab)) {
                wrist_C_Rotation += 1.0f;
                Update();
            }
        }
    }
};  


#endif
