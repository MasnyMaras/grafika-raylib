#ifndef ROBOT_H
#define ROBOT_H

#include "raylib.h"
#include "raymath.h"
#include "rlights.h"
#include <iostream>

const float ShoulderLength = 27.8f;
//klasa odpowiedzialna za generowanie, transformacje, rysowanie i unloadowanie poszczególnych części
class RobotPart {
public:
    Model model; //model częsci
    Vector3 position; //pozycja częsci
    // Matrix GetTransform() const { return model.transform; } //funkcja do pobierania transformacji

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
    //definiowanie części robota o konkretnych atrybutach z klasy RobotPart
    RobotPart waist;
    RobotPart shoulder;
    RobotPart arm;
    RobotPart base;
    RobotPart wrist_A;

    float pitch = 0.0f;     // Y-Axis (obrót)
    float roll = 0.0f;      // X-Axis (podnoszenie ramienia)
    float rollArm = 0.0f;   // X-Axis (zginanie przedramienia)
    float wristRotation = 0.0f; 

    //Pozycje startowe konkretnych elementów
    Vector3 basepos = {0.0f, 0.0f, 0.0f}; //pozycja bazy robota
    Vector3 waistPos = { 0.0f, 0.0f, 0.0f };
    Vector3 shoulderPos = { 0.0f, 0.0f, 0.0f };
    Vector3 armPos = {0.0f, 0.0f, 0.0f};
    Vector3 wrist_A_Pos = {0.0f, 0.0, 0.0f }; 

    struct DHParams {
        float theta, delta, lambda, alpha;
    };
    DHParams dhParams[6];  // parametry dla 6 przegubów
    Matrix jointTransforms[6];  // wynikowe macierze

    // Funkcja generująca macierz D-H
    Matrix DHMatrix(float theta, float delta, float lambda, float alpha) {
        float ct = cosf(theta), st = sinf(theta);
        float ca = cosf(alpha), sa = sinf(alpha);
        return Matrix{
            ct, -st*ca,  st*sa, lambda*ct,
            st,  ct*ca, -ct*sa, lambda*st,
            0,      sa,     ca,    delta,
            0,       0,      0,    1
        };
    }

    //konstrutor robota - tworzy siatki elementów, a potem całe modele
    Robot(Shader shader) {
        waist = RobotPart("src/Pieza2_new.obj", shader, waistPos); 
        base = RobotPart("src/Pieza1_new.obj", shader, basepos);//Baza robota
        shoulder = RobotPart("src/Pieza3_new.obj", shader, shoulderPos);
        arm = RobotPart("src/Pieza44.obj", shader, armPos);
        wrist_A = RobotPart("src/Pieza5.obj", shader, wrist_A_Pos);
    }
    //funkcja to praktycznie 1 do 1 to samo co było poprzednio przed główną pętlą while
    void Update() {
        
         
        
        // TEST 2: BIODRO + RAMIĘ
        
        // KROK 1: ORIENTACJE POCZĄTKOWE (zgodne z konwencją D-H)
        //Matrix baseOrientation = MatrixRotateX(DEG2RAD * -90.0f);
        //Matrix waistOrientation = MatrixRotateX(DEG2RAD * -90.0f);  
        //Matrix shoulderOrientation = MatrixRotateX(DEG2RAD * 180.0f);
        //Matrix armOrientation = MatrixRotateX(DEG2RAD * 180.0f);


        // Przegub 0: Biodro - obrót wokół Z
        dhParams[0].theta = DEG2RAD * pitch;    // A/D
        dhParams[0].delta = 0.0f;                   
        dhParams[0].alpha = 0.0f;           
        dhParams[0].lambda = DEG2RAD * 90.0f; 
        // Przegub 1: Ramię - połączone z biodrem
        dhParams[1].theta = DEG2RAD * roll;     // W/S
        dhParams[1].delta = 0.0f;
        dhParams[1].lambda = 10.0f;                  
        dhParams[1].alpha  = DEG2RAD * -90.0f;

        // Zerowanie pozostałych   
        for(int i = 2; i < 6; i++) {
            dhParams[i].theta = 0.0f;
            dhParams[i].delta = 0.0f;
            dhParams[i].lambda = 0.0f;
            dhParams[i].alpha = 0.0f;
        }

        // Łańcuchowe mnożenie - TYLKO 2 przeguby
        // Matrix T = MatrixIdentity();
        // for(int i = 0; i < 2; i++) {  // TYLKO i < 2
        //     Matrix A = DHMatrix(dhParams[i].theta, dhParams[i].d, 
        //                     dhParams[i].a, dhParams[i].alpha);
        //     T = MatrixMultiply(T, A);
        //     jointTransforms[i] = T;
        // }

         // POPRAWNE ŁAŃCUCHOWE MNOŻENIE
    jointTransforms[0] = DHMatrix(dhParams[0].theta, dhParams[0].delta, 
                                  dhParams[0].lambda, dhParams[0].alpha);
    
    jointTransforms[1] = MatrixMultiply(jointTransforms[0], 
                                        DHMatrix(dhParams[1].theta, dhParams[1].delta, 
                                                dhParams[1].lambda, dhParams[1].alpha));


        
        // USTAWIENIE
       // base.SetTransform(baseOrientation);
        waist.SetTransform(jointTransforms[0]);
        shoulder.SetTransform(jointTransforms[1]);
           

        /* TO CO BYŁO WCZEŚNIEJ PRZED DODANIEM MACIERZY D-H
        // ustawienie pozycji bazy robota -------------------------
        base.SetTransform(MatrixMultiply(MatrixRotateX(DEG2RAD*-90.0f), MatrixRotateY(DEG2RAD*90.0f))); // Obrót bazy robota 
        
        Matrix waistRotation = MatrixRotateY(DEG2RAD * pitch);
        Matrix initialWaistOrientation = MatrixMultiply(MatrixRotateX(DEG2RAD*90.0f), MatrixRotateY(DEG2RAD*90.0f)); 
        Matrix waistTranslation = MatrixTranslate(0.0f, 23.0f, 0.0f);
        Matrix waistTransform = MatrixMultiply(MatrixMultiply(initialWaistOrientation, waistRotation), waistTranslation); 
        waist.SetTransform(waistTransform);
        // aktualizacja pozycji biodra robota dla testu
        //waistPos = Vector3Transform(Vector3Zero(), waistTransform); // Ustawienie pozycji biodra
        //std::cout << "Waist Position: " << waistPos.x << ", " << waistPos.y << ", " << waistPos.z << std::endl;


        Matrix initialShoulderOrientation = MatrixMultiply(MatrixRotateX(DEG2RAD*-90.0f), MatrixRotateY(DEG2RAD*90.0f)); 
        Matrix shoulderOffset = MatrixTranslate(0.0f, 23.0f, 6.8f); 
        Matrix shoulderRoll = MatrixRotateX(DEG2RAD * -roll);
        Matrix shoulderTransform = MatrixMultiply(shoulderRoll, MatrixMultiply(initialShoulderOrientation, MatrixMultiply(shoulderOffset, waistRotation)));
        shoulder.SetTransform(shoulderTransform); // Ustawienie transformacji ramienia

        Matrix initialArmOrientation = MatrixRotateY(DEG2RAD *-90.0f); 
        Matrix armOffset = MatrixTranslate(0.0f, 23.0f, 3.8f); 

        Matrix armLift = MatrixTranslate(0.0f, sin(DEG2RAD * roll)*16.5f, -cos(DEG2RAD * roll)*16.5f); // Podnoszenie przedramienia
        Matrix armBend = MatrixRotateX(DEG2RAD * rollArm); // Zginanie przedramienia
        Matrix armTransform = MatrixMultiply(armBend, MatrixMultiply(armLift, MatrixMultiply(initialArmOrientation, MatrixMultiply(armOffset, waistRotation)))); // Łączenie transformacji
        arm.SetTransform(armTransform);
        // aktualizacja pozycji ramienia robota
        //armPos = Vector3Transform(Vector3Zero(), armTransform); 
        //std::cout << "Arm Position: " << armPos.x << ", " << armPos.y << ", " << armPos.z << std::endl;

        Matrix initialWrist_A_Orientation = MatrixRotateY(DEG2RAD * -90.0f); // Początkowa orientacja nadgarstka
        Matrix wrist_A_Offset = MatrixTranslate(31.2f, 23.0f, 5.3f); // Przesunięcie nadgarstka
        Matrix wrist_A_Transform = MatrixMultiply(initialWrist_A_Orientation, wrist_A_Offset);
        wrist_A.SetTransform(wrist_A_Transform); // Ustawienie transformacji nadgarstka
        */
    }

    //rysowanie elementów
    void Draw() {
        // base.Draw(LIGHTGRAY);
         //base.DrawAxes(30.0f); //rysowanie bazy robota
        waist.Draw(LIGHTGRAY);
        waist.DrawAxes(30.0f);
        shoulder.Draw(LIGHTGRAY);
        shoulder.DrawAxes(30.0f);
         //arm.Draw(LIGHTGRAY);
         //arm.DrawAxes(30.0f);
        // wrist_A.Draw(LIGHTGRAY); 
        // wrist_A.DrawAxes(100.0f);
    }

    //unloadowanie elementów
    void Unload() {
        waist.Unload();
        shoulder.Unload();
        arm.Unload();
        base.Unload();
        wrist_A.Unload();
    }
    //inputy do sterowania
    void HandleInput() {
        // if (IsKeyDown(KEY_A)){
        //     if(pitch<=180) pitch += 1.0f;} 
        // if (IsKeyDown(KEY_D)){
        //     if(pitch>=-140) pitch -= 1.0f;}
        // if (IsKeyDown(KEY_W)){
        //     if(roll<=133) roll += 1.0f;}
        // if (IsKeyDown(KEY_S)){;
        //     if(roll>=-133) roll -= 1.0f;}
        // if (IsKeyDown(KEY_UP)){
        //     if(roll<=142) rollArm += 1.0f;}
        // if (IsKeyDown(KEY_DOWN)){
        //      if(rollArm!=-270)rollArm -= 1.0f;}
             
        // if (IsKeyDown(KEY_RIGHT)){
        //     if(wristRotation<=90) wristRotation += 1.0f;}
        // if (IsKeyDown(KEY_LEFT)){
        //     if(wristRotation>=-90) wristRotation -= 1.0f;}
        // 
         if (IsKeyDown(KEY_A)){
             pitch += 1.0f;} 
        if (IsKeyDown(KEY_D)){
             pitch -= 1.0f;}
        if (IsKeyDown(KEY_W)){
            roll += 1.0f;}
        if (IsKeyDown(KEY_S)){;
            roll -= 1.0f;}
        if (IsKeyDown(KEY_UP)){
            rollArm += 1.0f;}
        if (IsKeyDown(KEY_DOWN)){
             rollArm -= 1.0f;}
        }
};

class ObjectModel {
public:
    Model model;
    Vector3 position;

    ObjectModel() {}
    ObjectModel(Mesh mesh, Shader shader, Vector3 pos) {
        model = LoadModelFromMesh(mesh);
        model.materials[0].shader = shader;
        position = pos;
    }
    void SetTransform(Matrix transform) { 
        model.transform = transform;
    }
    void Draw(Color tint) {
        DrawModel(model, position, 1.0f, tint);
    }
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
    void Unload() {
        UnloadModel(model);
    }
};

class Object {
    public: 
    ObjectModel cube;
    ObjectModel sphere;
    Vector3 cubePos = {0.0f, 0.0f, 00.0f}; //pozycja sześcianu
    Vector3 spherePos = {00.0f, 0.0f, 00.0f}; //pozycja kuli, póki co nieużywana

    Object(Shader shader) {
        Mesh mesh = GenMeshCube(3.0f, 3.0f, 3.0f); //generowanie sześcianu
        cube = ObjectModel(mesh, shader, cubePos); //tworzenie modelu sześcianu
        Mesh sphereMesh = GenMeshSphere(1.5f, 16, 16); //generowanie kuli
        sphere = ObjectModel(sphereMesh, shader, spherePos); //tworzenie modelu kuli
    }
    void Update() {
        Matrix cubeTransform = MatrixTranslate(35.0f, 0.0f, 0.0f); //przesunięcie sześcianu
        Matrix cubeRotation = MatrixRotateY(DEG2RAD * 90.0f); //obrót sześcianu
        cube.SetTransform(MatrixMultiply(cubeRotation, cubeTransform)); 
        //cube.SetTransform(MatrixMultiply(cubeTransform, cubeRotation)); //takie tam testowanie

    }
    void Draw() {
        //cube.Draw(BLUE); //rysowanie sześcianu
        //cube.DrawAxes(10.0f); //rysowanie osi sześcianu
        //sphere.Draw(RED); //rysowanie kuli
    }
    void Unload() {
        cube.Unload(); //zwalnianie pamięci
        sphere.Unload(); //zwalnianie pamięci
    }
};





#endif
