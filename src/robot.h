#ifndef ROBOT_H
#define ROBOT_H

#include "raylib.h"
#include "raymath.h"
#include "rlights.h"
const float WaistRadius = 0.8f;
const float WaistHeight = 3.0f;
const float ShoulderWidth = 0.5f;
const float ShoulderLength = 0.5f;
const float ShoulderHeight = 3.0f;
const float ArmWidth = 0.25f;
const float ArmLength = 0.25f;
const float ArmHeight = 2.8f;
//klasa odpowiedzialna za generowanie, transformacje, rysowanie i unloadowanie poszczególnych części
class RobotPart {
public:
    Model model; //model częsci
    Vector3 position; //pozycja częsci

    RobotPart() {}
    
    //Konstruktor - tworzy model, daje materiał do shadera i pozycje obiektu
    RobotPart(Mesh mesh, Shader shader, Vector3 pos) {      //Każda część robota składa się z wykorzystywana do stworzenia modelu składa się z siatki, shadera i pozycji
        model = LoadModelFromMesh(mesh);        //na bazie siatki tworzymy model
        model.materials[0].shader = shader;     //Dajemy elementom "Materiał" shadera - to jak bedzie odbijał światło
        position = pos;                         //deklaracja pozycji
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

    float pitch = 0.0f;     // Y-Axis (obrót)
    float roll = 0.0f;      // X-Axis (podnoszenie ramienia)
    float rollArm = 0.0f;   // X-Axis (zginanie przedramienia)

    //Pozycje startowe konkretnych elementów
    Vector3 waistPos = { 0.0f, 0.0f, 0.0f };
    Vector3 shoulderPos = { WaistRadius + ShoulderWidth/2, 0.8f*WaistHeight, -ShoulderHeight / 2.0f };
    Vector3 armPos = { WaistRadius + ShoulderWidth + ArmWidth/2, 0.8f*WaistHeight, -1.45f * ShoulderHeight + ArmWidth};  // Z = -1.45 * 3.0

    //konstrutor robota - tworzy siatki elementów, a potem całe modele
    Robot(Shader shader) {
        Mesh waistMesh = GenMeshCylinder(WaistRadius, WaistHeight, 48); //generacja siatki bioder z rayliba
        waist = RobotPart(waistMesh, shader, waistPos); //generacja modelu bioder z klasy RobotPart do której podajemy siatkę, shadera i pozycję
                                                        //I teraz mamy gotowy model bioder, którego możemy używać później ***

        Mesh shoulderMesh = GenMeshCube(ShoulderWidth, ShoulderLength, ShoulderHeight); //generacja siatki ramienia z rayliba
        shoulder = RobotPart(shoulderMesh, shader, shoulderPos); //generacja modelu ramienia z klasy RobotPart do której podajemy siatkę, shadera i pozycję

        Mesh armMesh = GenMeshCube(ArmWidth, ArmLength, ArmHeight);  //generacja siatki przedramienia z rayliba
        arm = RobotPart(armMesh, shader, armPos); //generacja modelu przedramienia z klasy RobotPart do której podajemy siatkę, shadera i pozycję
    }
    //funkcja to praktycznie 1 do 1 to samo co było poprzednio przed główną pętlą while
    void Update() {
        // Obrót bioder
        Matrix waistRot = MatrixRotateY(DEG2RAD * pitch);
        Matrix waistTransform = MatrixMultiply(MatrixTranslate(waistPos.x, waistPos.y, waistPos.z), waistRot);
        waist.SetTransform(waistTransform); //*** np tutaj odnosimy sie do stworzonych wcześniej bioder i odpowiednio je przerabiamy

        // Transformacja Ramienia
        Matrix shoulderRotY = MatrixMultiply(
            MatrixTranslate(shoulderPos.x, 0.0f, shoulderPos.z),
            MatrixMultiply(MatrixRotateY(DEG2RAD * pitch),
            MatrixTranslate(-shoulderPos.x, 0.0f, -shoulderPos.z))
        );
        //Podnoszenie ramienia
        Matrix shoulderRotX = MatrixMultiply(
            MatrixTranslate(shoulderPos.x, 0.0f, shoulderPos.z),
            MatrixMultiply(MatrixRotateX(DEG2RAD * roll),
            MatrixTranslate(-shoulderPos.x, 0.0f, -shoulderPos.z))
        );

        shoulder.SetTransform(MatrixMultiply(shoulderRotX, shoulderRotY)); //Podanie zmian do modelu ramienia

        // Transformacja Przedramienia
        Matrix armRotY = MatrixMultiply(
            MatrixTranslate(armPos.x, 0.0f, armPos.z),
            MatrixMultiply(MatrixRotateY(DEG2RAD * pitch),
            MatrixTranslate(-armPos.x, 0.0f, -armPos.z))
        );

        // Podnoszenie przedramienia razem z ramieniem
        Matrix armLift = MatrixTranslate(0.0f, sin(DEG2RAD * roll) * shoulderPos.y + sin(DEG2RAD * roll)*0.2f*WaistHeight, -2 * shoulderPos.z - cos(DEG2RAD * roll) * shoulderPos.y - cos(DEG2RAD * roll)*0.2f*WaistHeight);

        Matrix armMoved = MatrixMultiply(armLift, armRotY);

        // zginanie przedramienia
        Matrix armBend = MatrixMultiply(
            MatrixTranslate(0.0f , 0.0f, armPos.z + ArmHeight + ShoulderWidth/2),
            MatrixMultiply(MatrixRotateX(DEG2RAD * rollArm),
            MatrixTranslate(0.0f , 0.0f, -armPos.z - ArmHeight - ShoulderWidth/2))
        );

        arm.SetTransform(MatrixMultiply(armBend, armMoved)); //podanie zmian do modelu
    }
    //rysowanie elementów
    void Draw() {
        waist.Draw(RED);
        shoulder.Draw(BLUE);
        arm.Draw(GREEN);
    }
    //unloadowanie elementów
    void Unload() {
        waist.Unload();
        shoulder.Unload();
        arm.Unload();
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

#endif
