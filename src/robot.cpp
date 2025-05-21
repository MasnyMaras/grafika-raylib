#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <cmath>

int main() {
    const int screenWidth = 1960;
    const int screenHeight = 1084;
    const float WaistRadius = 0.8f;
    const float WaistHeight = 3.0f;
    const float ShoulderWidth = 0.5f;
    const float ShoulderLength = 0.5f;
    const float ShoulderHeight = 3.0f;
    const float ArmWidth = 0.25f;
    const float ArmLength = 0.25f;
    const float ArmHeight = 2.8f;
    
    InitWindow(screenWidth, screenHeight, "KAROL - The BOKSER");

    // Kamera ustawiona na stałe
    Camera3D camera = { 0 };
    camera.position = { 10.0f, 10.0f, 10.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };       
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Model bioder
    Mesh waistMesh = GenMeshCylinder(WaistRadius, WaistHeight, 48);
    Model waistModel = LoadModelFromMesh(waistMesh);

    // Model ramienia
    Mesh ShoulderMesh = GenMeshCube(ShoulderWidth, ShoulderLength, ShoulderHeight);
    Model ShoulderModel = LoadModelFromMesh(ShoulderMesh);

    //model przedramienia
    Mesh ArmMesh = GenMeshCube(ArmWidth, ArmLength, ArmHeight);
    Model ArmModel = LoadModelFromMesh(ArmMesh);
    // Parametry do obrotu i pozycjonowania
    float pitch = 0.0f; // Y-Axis
    float rollArm = 0.0f; //Y-Axis dla przedramienia
    float roll = 0.0f; // X-Axis

    Vector3 WaistPosition = { 0.0f, 0.0f, 0.0f };
    Vector3 ShoulderPosition = { 0.0f, WaistHeight, (-ShoulderHeight / 2) };
    Vector3 ArmPosition = { 0.0f, WaistHeight, (-1.45*ShoulderHeight) };
    
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Obrót bioder
        if (IsKeyDown(KEY_A)) pitch += 1.0f;
        else if (IsKeyDown(KEY_D)) pitch -= 1.0f;
        if (IsKeyDown(KEY_W)) roll += 1.0f;
        else if (IsKeyDown(KEY_S)) roll -= 1.0f;
        if (IsKeyDown(KEY_UP)) rollArm += 1.0f;
        else if (IsKeyDown(KEY_DOWN)) rollArm -= 1.0f;


        // === Transformacja bioder ===
        Matrix waistRotation = MatrixRotateY(DEG2RAD * pitch);
        Matrix waistTranslation = MatrixTranslate(WaistPosition.x, WaistPosition.y, WaistPosition.z);
        Matrix waistTransform = MatrixMultiply(waistTranslation, waistRotation);
        waistModel.transform = waistTransform;
        // obrot ramienia
        Matrix shoulderTranslation = MatrixTranslate(ShoulderPosition.x, 0.0f, ShoulderPosition.z); //przesuwamy element na środek układu
        Matrix ShoulderRotation = MatrixRotateY(DEG2RAD*pitch); // obracamy 
        Matrix shoulderTranslationBack = MatrixTranslate(-ShoulderPosition.x, 0.0f, -ShoulderPosition.z); // wracamy na pierwotne miejsce
        Matrix SchoulderTransform = MatrixMultiply(MatrixMultiply(shoulderTranslation, ShoulderRotation), shoulderTranslationBack); //mnożymy obrót w środku, żeby odpowiednio ustawić element -> cofamy do pierwotnego położenia
        // Obrót przedramienia
        Matrix ArmTranslation = MatrixTranslate(ArmPosition.x, 0.0f, ArmPosition.z);
        Matrix ArmRotation = MatrixRotateY(DEG2RAD*pitch);
        Matrix ArmTranslationBack = MatrixTranslate(-ArmPosition.x, 0.0f, -ArmPosition.z);
        Matrix ArmTransform = MatrixMultiply(MatrixMultiply(ArmTranslation, ArmRotation), ArmTranslationBack);
        
        ArmModel.transform = ArmTransform;
        // Dodaj obrót ramienia w osi X (podnoszenie)
        Matrix shoulderLiftOrigin = MatrixTranslate(ShoulderPosition.x, 0.0f, ShoulderPosition.z);
        Matrix shoulderLiftBack = MatrixTranslate(-ShoulderPosition.x, 0.0f, -ShoulderPosition.z);
        Matrix shoulderLiftRotation = MatrixRotateX(DEG2RAD * roll);
        // Transformacja uwzględniająca zarówno obrót ramienia wokół Y (biodra), jak i X (podnoszenie)
        Matrix shoulderLiftTransform = MatrixMultiply(MatrixMultiply(shoulderLiftOrigin, shoulderLiftRotation),shoulderLiftBack);
        // Połącz obie transformacje
        ShoulderModel.transform = MatrixMultiply(shoulderLiftTransform, SchoulderTransform);
        // Dodaj obrót przedramienia w osi X (podnoszenie)
        Matrix armRaise = MatrixTranslate(0.0f, sin(DEG2RAD*roll)*ShoulderPosition.y, -2*ShoulderPosition.z-cos(DEG2RAD*roll)*ShoulderPosition.y);
        Matrix armRiseTransform = MatrixMultiply(armRaise, ArmTransform);
        //ArmModel.transform = armRiseTransform;
        //Zginanie przedramienia
        Matrix ArmLiftOrigin = MatrixTranslate(ArmPosition.x + ArmHeight, 0.0f, ArmPosition.z + ArmHeight );
        Matrix ArmLiftRotation = MatrixRotateX(DEG2RAD * rollArm);
        Matrix ArmLiftBack = MatrixTranslate(-ArmPosition.x - ArmHeight, 0.0f, -ArmPosition.z - ArmHeight);
        Matrix ArmLiftTransform = MatrixMultiply(MatrixMultiply(ArmLiftOrigin,ArmLiftRotation), ArmLiftBack);
        ArmModel.transform = MatrixMultiply(ArmLiftTransform, armRiseTransform);
        
        //ArmModel.transform = ArmLiftTransform;
        // Rysowanie
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
        
        DrawModel(waistModel, WaistPosition, 1.0f, RED); // Biodra
        DrawModel(ShoulderModel, ShoulderPosition, 1.0f, BLUE); // Ramię
        DrawModel(ArmModel, ArmPosition, 1.0f, GREEN); //przedramię

        // Siatka na robocie
        rlEnableWireMode();
        DrawModel(waistModel, WaistPosition, 1.0f, GRAY); // Biodra
        DrawModel(ShoulderModel, ShoulderPosition, 1.0f, GRAY); // Ramię
        DrawModel(ArmModel, ArmPosition, 1.0f, GRAY); //przedramię
        rlDisableWireMode();

        DrawGrid(1000, 1.0f); // Siatka
        EndMode3D();
        DrawText("KAROL - The BOKSER", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
