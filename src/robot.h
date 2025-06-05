#ifndef ROBOT_H
#define ROBOT_H

#include "raylib.h"
#include "raymath.h"
#include "rlights.h"
const float ShoulderLength = 27.8f;
//klasa odpowiedzialna za generowanie, transformacje, rysowanie i unloadowanie poszczególnych części
class RobotPart {
public:
    Model model; //model częsci
    Vector3 position; //pozycja częsci
    Matrix GetTransform() const { return model.transform; } //funkcja do pobierania transformacji

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
        DrawModel(model, position, 1.0f, tint);
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
    Vector3 wrist_A_Pos = {12.3f, waistPos.y, -29.1f }; 

    //konstrutor robota - tworzy siatki elementów, a potem całe modele
    Robot(Shader shader) {
        waist = RobotPart("src/Pieza2.obj", shader, waistPos); 
        base = RobotPart("src/Pieza1.obj", shader, basepos);//Baza robota
        shoulder = RobotPart("src/Pieza3.obj", shader, shoulderPos);
        arm = RobotPart("src/Pieza4.obj", shader, armPos);
        wrist_A = RobotPart("src/Pieza5.obj", shader, wrist_A_Pos);
    }
    //funkcja to praktycznie 1 do 1 to samo co było poprzednio przed główną pętlą while
    void Update() {
        
        // ustawienie pozycji bazy robota -------------------------
        base.SetTransform(MatrixMultiply(MatrixRotateX(DEG2RAD*-90.0f), MatrixRotateY(DEG2RAD*90.0f))); // Obrót bazy robota 
        // --------------------------------------------------------

        
        Matrix waistRotation = MatrixRotateY(DEG2RAD * pitch);
        Matrix initialWaistOrientation = MatrixMultiply(MatrixRotateX(DEG2RAD*90.0f), MatrixRotateY(DEG2RAD*90.0f)); 
        Matrix waistTranslation = MatrixTranslate(0.0f, 23.0f, 0.0f);
        Matrix waistTransform = MatrixMultiply(MatrixMultiply(initialWaistOrientation, waistRotation), waistTranslation); 
        waist.SetTransform(waistTransform); 


        Matrix initialShoulderOrientation = MatrixMultiply(MatrixRotateX(DEG2RAD*-90.0f), MatrixRotateY(DEG2RAD*90.0f)); 
        Matrix shoulderOffset = MatrixTranslate(0.0f, 23.0f, 6.8f); 
        Matrix shoulderRoll = MatrixRotateX(DEG2RAD * -roll);
        Matrix shoulderTransform = MatrixMultiply(shoulderRoll, MatrixMultiply(initialShoulderOrientation, MatrixMultiply(shoulderOffset, waistRotation)));
        shoulder.SetTransform(shoulderTransform); // Ustawienie transformacji ramienia

        Matrix initialArmOrientation = MatrixRotateZ(DEG2RAD * (0.0f)); 
        arm.SetTransform(initialArmOrientation);




         /*
        // to co było wcześniej 
        // Transformacja Bioder -----------------------------------
        Matrix waistRotY = MatrixRotateY(DEG2RAD * (pitch + 90.0f)); // Obrót bioder wokół osi Y, +90 stopni dla poprawnej orientacji
        Matrix waistRotX = MatrixRotateX(DEG2RAD * 90.0f); // Obrót o 90 stopni wokół osi X, dla poprawnej orientacji
        Matrix waistRotation = MatrixMultiply(waistRotX, waistRotY); // Łączenie obrotów
        Matrix waistTransform = MatrixMultiply(waistRotation, MatrixTranslate(waistPos.x, 0.0f, waistPos.z)); // Transformacja bioder
        // ostateczna transformacja bioder
        waist.SetTransform(waistTransform); //*** np tutaj odnosimy sie do stworzonych wcześniej bioder i odpowiednio je przerabiamy
        // --------------------------------------------------------

        // Transformacja Ramienia -----------------------------------
        Matrix shoulderRotY = MatrixMultiply(                               // obrot w osi Y
            MatrixTranslate(shoulderPos.x, shoulderPos.y, shoulderPos.z),
            MatrixMultiply(MatrixRotateY(DEG2RAD * (pitch + 90.0f)), // obrot, razem z biodrami, +90 dla poprawnego startu
            MatrixTranslate(-shoulderPos.x, -shoulderPos.y, -shoulderPos.z))
        );
        Matrix shoulderRotX = MatrixMultiply(               // obrot w osi X
            MatrixTranslate(waistPos.x, 0.0f, waistPos.z), //korzystamy z waistPos, bo to jest punkt odniesienia dla ramienia, które jest niesymetryczne
            MatrixMultiply(MatrixRotateX(DEG2RAD * (-roll - 90.0f)), // podnoszenie i oposzczanie, -90 dla poprawnego startu
            MatrixTranslate(-waistPos.x, 0.0f, -waistPos.z))
        );
        // Finalna transformacja dla ramienia
        Matrix shoulderRotation = MatrixMultiply(shoulderRotX, shoulderRotY); // Łączenie obrotów ramienia
        shoulder.SetTransform(shoulderRotation); // podanie zmian do modelu
        // --------------------------------------------------------

        // Transformacja Przedramienia -----------------------------------
        Matrix armRotY = MatrixMultiply(
            MatrixTranslate(armPos.x, 0.0f, armPos.z),
            MatrixMultiply(MatrixRotateY(DEG2RAD * (pitch-90.0f)), // obrót przedramienia wokół osi Y, -90 dla poprawnego startu
            MatrixTranslate(-armPos.x, 0.0f, -armPos.z))
        );

        // Podnoszenie przedramienia razem z ramieniem
        Matrix armLift = MatrixTranslate(0.0f, sin(DEG2RAD * roll)*(ShoulderLength-armPos.x), - cos(DEG2RAD * roll) * (ShoulderLength - armPos.x)); // podnoszenie przedramienia, zależne od kąta roll
        Matrix armMoved = MatrixMultiply(armLift, armRotY);

        // zginanie przedramienia
        Matrix armBend = MatrixMultiply(
            MatrixTranslate(0.0f , 0.0f, armPos.z),
            MatrixMultiply(MatrixRotateX(DEG2RAD * rollArm),
            MatrixTranslate(0.0f, 0.0f, -armPos.z))
        );

        arm.SetTransform(MatrixMultiply(armBend, armMoved)); //podanie zmian do modelu
        // --------------------------------------------------------

        // Transformacja Nadgarstka -----------------------------------
        Matrix wristRotY = MatrixMultiply(
            MatrixTranslate(wrist_A_Pos.x, wrist_A_Pos.y, wrist_A_Pos.z),
            MatrixMultiply(MatrixRotateY(DEG2RAD * (pitch - 90.0f)), // obrót nadgarstka wokół osi Y, -90 dla poprawnego startu
            MatrixTranslate(-wrist_A_Pos.x, -wrist_A_Pos.y, -wrist_A_Pos.z))
        );

        Matrix wristRotZ = MatrixMultiply(
            MatrixTranslate(0.0f, 0.0f, wrist_A_Pos.z - armPos.z), // przesunięcie w osi Z, aby nadgarstek był na końcu przedramienia
            MatrixMultiply(MatrixRotateZ(DEG2RAD * wristRotation), // obrót nadgarstka wokół osi X
            MatrixTranslate(0.0f, 0.0f, -(wrist_A_Pos.z - armPos.z))) // przesunięcie w osi Z, aby nadgarstek był na końcu przedramienia
        );

        wrist_A.SetTransform(MatrixMultiply(wristRotZ, wristRotY)); 
        */
        
    }

    //rysowanie elementów
    void Draw() {
        //base.Draw(LIGHTGRAY);
        waist.Draw(LIGHTGRAY);
        //waist.DrawAxes(100.0f);
        shoulder.Draw(LIGHTGRAY);
        //shoulder.DrawAxes(100.0f);
        arm.Draw(LIGHTGRAY);
        arm.DrawAxes(100.0f);
        //wrist_A.Draw(LIGHTGRAY); 

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
        if (IsKeyDown(KEY_A)){
            if(pitch<=180) pitch += 1.0f;} 
        if (IsKeyDown(KEY_D)){
            if(pitch>=-140) pitch -= 1.0f;}
        if (IsKeyDown(KEY_W)){
            if(roll<=133) roll += 1.0f;}
        if (IsKeyDown(KEY_S)){;
            if(roll>=-133) roll -= 1.0f;}
        if (IsKeyDown(KEY_UP)){
            if(roll<=142) rollArm += 1.0f;}
        if (IsKeyDown(KEY_DOWN)){
             if(roll<=142)rollArm -= 1.0f;}
        if (IsKeyDown(KEY_RIGHT)){
            if(wristRotation<=90) wristRotation += 1.0f;}
        if (IsKeyDown(KEY_LEFT)){
            if(wristRotation>=-90) wristRotation -= 1.0f;}
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
    void Unload() {
        UnloadModel(model);
    }
};

class Object {
    public: 
    ObjectModel cube;
    ObjectModel sphere;
    Vector3 cubePos = {20.0f, 0.0f, 20.0f}; //pozycja sześcianu
    Vector3 spherePos = {30.0f, 0.0f, 30.0f}; //pozycja kuli, póki co nieużywana

    Object(Shader shader) {
        Mesh mesh = GenMeshCube(3.0f, 3.0f, 3.0f); //generowanie sześcianu
        cube = ObjectModel(mesh, shader, cubePos); //tworzenie modelu sześcianu
        Mesh sphereMesh = GenMeshSphere(1.5f, 16, 16); //generowanie kuli
        sphere = ObjectModel(sphereMesh, shader, spherePos); //tworzenie modelu kuli
    }
    void Update() {
        // Można dodać logikę aktualizacji pozycji sześcianu, jeśli jest taka potrzeba
    }
    void Draw() {
        cube.Draw(BLUE); //rysowanie sześcianu
        sphere.Draw(RED); //rysowanie kuli
    }
    void Unload() {
        cube.Unload(); //zwalnianie pamięci
        sphere.Unload(); //zwalnianie pamięci
    }
};





#endif
