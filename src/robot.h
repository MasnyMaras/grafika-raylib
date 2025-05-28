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
};

//reprezentacja całego robota
class Robot {
public:
    //definiowanie części robota o konkretnych atrybutach z klasy RobotPart
    RobotPart waist;
    RobotPart shoulder;
    RobotPart arm;
    RobotPart base;

    float pitch = 0.0f;     // Y-Axis (obrót)
    float roll = 0.0f;      // X-Axis (podnoszenie ramienia)
    float rollArm = 0.0f;   // X-Axis (zginanie przedramienia)

    //Pozycje startowe konkretnych elementów
    Vector3 basepos = {0.0f, 0.0f, 0.0f}; //pozycja bazy robota
    Vector3 waistPos = { 0.0f, 23.0f, 0.0f };
    Vector3 shoulderPos = { -6.8f, waistPos.y, 0.0f };
    Vector3 armPos = {10.8f, waistPos.y, 2.5f};

    //konstrutor robota - tworzy siatki elementów, a potem całe modele
    Robot(Shader shader) {
        waist = RobotPart("src/Pieza2.obj", shader, waistPos); 
        base = RobotPart("src/Pieza1.obj", shader, basepos);//Baza robota
        shoulder = RobotPart("src/Pieza3.obj", shader, shoulderPos);
        arm = RobotPart("src/Pieza456.obj", shader, armPos);
    }
    //funkcja to praktycznie 1 do 1 to samo co było poprzednio przed główną pętlą while
    void Update() {
        
        // ustawienie pozycji bazy robota -------------------------
        base.SetTransform(MatrixRotateX(DEG2RAD*-90.0f)); // Obrót bazy robota o -90 stopni wokół osi X
        // --------------------------------------------------------

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

    }
    //rysowanie elementów
    void Draw() {
        base.Draw(LIGHTGRAY);
        waist.Draw(LIGHTGRAY);
        shoulder.Draw(LIGHTGRAY);
        arm.Draw(LIGHTGRAY);

    }
    //unloadowanie elementów
    void Unload() {
        waist.Unload();
        shoulder.Unload();
        arm.Unload();
        base.Unload();
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
