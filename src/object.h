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
    ObjectModel sphere;
    Vector3 cubePos = {0.0f, 0.0f, 0.0f}; //pozycja sześcianu
    Vector3 spherePos = {0.0f, 0.0f, 0.0f}; //pozycja kuli
    bool grab = false;
    bool grabbingCube = false;    
    bool grabbingSphere = false;

    Object(Shader shader) {
        Mesh mesh = GenMeshCube(3.0f, 3.0f, 3.0f); //generowanie sześcianu
        cube = ObjectModel(mesh, shader, cubePos); //tworzenie modelu sześcianu
        Mesh sphereMesh = GenMeshSphere(1.5f, 16, 16); //generowanie kuli
        sphere = ObjectModel(sphereMesh, shader, spherePos); //tworzenie modelu kuli
    }
    void Initialize() {
        cube.SetTransform(MatrixTranslate(20.0f, 0.0f, -20.0f)); 
        cubePos = {20.0f, 0.0f, -20.0f};
        sphere.SetTransform(MatrixTranslate(20.0f, 0.0f, 20.0f));
        spherePos = {20.0f, 0.0f, 20.0f};
    }

    void Update(Matrix endEffectorPos, bool gripperType) {
        if (grab) {
            // Jeśli jeszcze nic nie trzymamy, sprawdź co można chwycić
            if (!grabbingCube && !grabbingSphere) {
                if (IsNearCube(endEffectorPos)) {
                    grabbingCube = true;  // Zacznij trzymać kostkę
                }
                else if (IsNearSphere(endEffectorPos) && gripperType) {
                    grabbingSphere = true;  // Zacznij trzymać kulę
                }
            }
            
            // Poruszaj tylko tym obiektem który trzymamy
            if (grabbingCube) {
                Matrix cubeTransform = MatrixMultiply(MatrixTranslate(0.0f, 1.8f, 0.0f), endEffectorPos); 
                cube.SetTransform(cubeTransform);
                cubePos = {cubeTransform.m12, cubeTransform.m13, cubeTransform.m14};
            }
            if (grabbingSphere && gripperType) {
                Matrix sphereTransform = MatrixMultiply(MatrixTranslate(0.0f, 1.8f, 0.0f), endEffectorPos); 
                sphere.SetTransform(sphereTransform);
                spherePos = {sphereTransform.m12, sphereTransform.m13, sphereTransform.m14};
            }
        } else {
            // Gdy grab = false, puść wszystko
            grabbingCube = false;
            grabbingSphere = false;
        }
    }

    bool IsNearCube(Matrix endEffectorPos) {
        return (fabs(cubePos.x - endEffectorPos.m12) < 3.5f && 
                fabs(cubePos.y - endEffectorPos.m13) < 3.5f && 
                fabs(cubePos.z - endEffectorPos.m14) < 3.5f);
    }

    bool IsNearSphere(Matrix endEffectorPos) {
        return (fabs(spherePos.x - endEffectorPos.m12) < 3.5f && 
                fabs(spherePos.y - endEffectorPos.m13) < 3.5f && 
                fabs(spherePos.z - endEffectorPos.m14) < 3.5f);
    }

    void Draw() {
        cube.Draw(BLUE); //rysowanie sześcianu
        sphere.Draw(RED); //rysowanie kuli
    }
    void Unload() {
        cube.Unload(); //zwalnianie pamięci
        sphere.Unload(); //zwalnianie pamięci
    }
    void Input() {
        if (IsKeyPressed(KEY_SPACE)) {
            grab = !grab; //przełączanie stanu chwytania sześcianu 
        }
    }
    
    bool IsGrabbed()  {
        return grab; //zwracanie stanu chwytania
    }
    
};

#endif