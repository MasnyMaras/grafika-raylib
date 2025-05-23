#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "rlights.h"
#define RLIGHTS_IMPLEMENTATION
#include "robot.h"
#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION 330
#else
    #define GLSL_VERSION 100
#endif

int main() {

    //rozmiar okna
    const int screenWidth = 1960;
    const int screenHeight = 1084;

    // Włączenie antyaliasingu
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "KAROL - The BOKSER!");

    // Kamera ustawiona na stałe
    Camera3D camera = { 0 };
    camera.position = { 10.0f, 10.0f, 10.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    //wcytywanie shaderów do óswietlania
    Shader shader = LoadShader(
        TextFormat("C:/raylib/raylib/examples/shaders/resources/shaders/glsl330/lighting.vs", GLSL_VERSION),
        TextFormat("C:/raylib/raylib/examples/shaders/resources/shaders/glsl330/lighting.fs", GLSL_VERSION)
    );
    // Ustawienie lokalizacji "viewPos" w shaderze – pozycja kamery
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

    //ustawianie świateł otoczenia "ambient"
    SetShaderValue(shader, GetShaderLocation(shader, "ambient"), (float[4]){0.2f, 0.2f, 0.2f, 1.0f}, SHADER_UNIFORM_VEC4);

    //konfiguracja światła
    Vector3 LightPos = {1, 6, 2}; //źródło światła
    Light lights[MAX_LIGHTS] = {0};  //tablica świateł, póki co jest jedno, ale może sie pokusimy o różny rodzaj oświetlenia :)
    lights[0] = CreateLight(LIGHT_POINT, LightPos, Vector3Zero(), WHITE, shader); //parametry światła (punktowe)

    //wczytujemy robota z załadowanym wyżej shaderem
    Robot robot(shader);

    //generowanie podłoża
    Mesh groundMesh = GenMeshPlane(200.0f, 200.0f, 1, 1);
    Model groundModel = LoadModelFromMesh(groundMesh);
    Vector3 groundPosition = { 0.0f, 0.0f, 0.0f };

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        robot.HandleInput();    //klawisze do sterowania
        robot.Update();     //aktualizujemy pozycje i obrot robota
        UpdateLightValues(shader, lights[0]);   //akrualizujemy światło

        //zaczynamy rysowanie
        BeginDrawing();
            ClearBackground(RAYWHITE); //tło

            BeginMode3D(camera);    //zaczynamy rysowanie 3D
                BeginShaderMode(shader);    //zaczynamy rysowaniez shaderami
                    robot.Draw();       //rysujemy robota
                    DrawModel(groundModel, groundPosition, 1.0f, {198, 209, 252}); //rysujemy podłoże
                EndShaderMode();    //kończymy rysowanie z shaderami
            EndMode3D();            // i ryswoanie 3D

        DrawText("KAROL - The BOKSER", 10, 10, 20, DARKGRAY); //Tekst w lewym górnym rogu
        EndDrawing();
    }
    //zwalnianie pamięci
    robot.Unload();
    UnloadModel(groundModel);
    UnloadShader(shader);
    CloseWindow();
    return 0;
}
