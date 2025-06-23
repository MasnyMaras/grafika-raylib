#ifndef OBJECT_H
#define OBJECT_H

#include "raylib.h"
#include "raymath.h"
//#include "rlights.h"  chat kazał wywalić
#include <iostream>

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
    Vector3 cubePos = {0.0f, 0.0f, 0.0f}; //pozycja sześcianu
    bool grab = false; 

    Object(Shader shader) {
        Mesh mesh = GenMeshCube(3.0f, 3.0f, 3.0f); //generowanie sześcianu
        cube = ObjectModel(mesh, shader, cubePos); //tworzenie modelu sześcianu
    }
    void Initialize() {
        cube.SetTransform(MatrixTranslate(20.0f, 0.0f, -20.0f)); 
        cubePos = {20.0f, 0.0f, -20.0f};
    }
    void Update(Matrix endEffectorPos) {
        if (CheckContact(endEffectorPos)) {
            Matrix cubeTransform = MatrixMultiply(MatrixTranslate(0.0f, 1.7f, 0.0f), endEffectorPos); 
            cube.SetTransform(cubeTransform);
            cubePos = {cubeTransform.m12, cubeTransform.m13, cubeTransform.m14}; //aktualizacja pozycji sześcianu
        }

         


    }
    bool CheckContact(Matrix endEffectorPos){
        //std::cout << dd << std::endl; //debugowanie stanu chwytania
        if (fabs(cubePos.x - endEffectorPos.m12) < 3.0f && 
            fabs(cubePos.y - endEffectorPos.m13) < 3.0f && 
            fabs(cubePos.z - endEffectorPos.m14) < 3.0f && 
            grab) { //sprawdzenie kontaktu z końcówką robota
            return true; //kontakt z końcówką robota
        }
        return false;
    }
    void Draw() {
        cube.Draw(BLUE); //rysowanie sześcianu
        cube.DrawAxes(10.0f); //rysowanie osi sześcianu
    }
    void Unload() {
        cube.Unload(); //zwalnianie pamięci
    }
    void Input() {
        if (IsKeyPressed(KEY_SPACE)) {
            grab = !grab; //przełączanie stanu chwytania sześcianu 
        }
    }
};

#endif