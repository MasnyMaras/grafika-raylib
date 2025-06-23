#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "rlights.h"
#define RLIGHTS_IMPLEMENTATION
#include "robot.h"
#include "object.h"
#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION 330
#else
    #define GLSL_VERSION 100
#endif

int main() {

    //rozmiar okna
    const int screenWidth = 1500;
    const int screenHeight = 800;

    // Włączenie antyaliasingu
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "KAROL - The BOKSER!");

    // Kamera ustawiona na stałe
    float camAngleX = 45.0f * DEG2RAD;  // obrót poziomy
    float camAngleY = 45.0f * DEG2RAD;  // obrót pionowy
    float camDistance = 120.0f;         // odległość od środka
    Vector3 camTarget = { 0.0f, 0.0f, 0.0f };  // punkt, wokół którego kamera orbituje
    Camera3D camera = { 0 };
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
    Vector3 LightPos = {20, 50, 30}; //źródło światła
    Light lights[MAX_LIGHTS] = {0};  //tablica świateł, póki co jest jedno, ale może sie pokusimy o różny rodzaj oświetlenia :)
    lights[0] = CreateLight(LIGHT_POINT, LightPos, Vector3Zero(), WHITE, shader); //parametry światła (punktowe)

    //wczytujemy robota z załadowanym wyżej shaderem
    Robot robot(shader);
    Object object(shader); //przykładowy obiekt do testowania
    object.Initialize(); //inicjalizacja sześcianu

    //generowanie podłoża
    Mesh groundMesh = GenMeshPlane(200.0f, 200.0f, 1, 1);
    Model groundModel = LoadModelFromMesh(groundMesh);
    Vector3 groundPosition = { 0.0f, 0.0f, 0.0f };
    Matrix endEffectorTransform = MatrixIdentity();


    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        robot.HandleInput();    //klawisze do sterowania
        object.Input(); //klawisz do chwytania sześcianu
        robot.Update();     //aktualizujemy pozycje i obrot robota
            endEffectorTransform = robot.jointTransforms[6]; //aktualizujemy transformację końcówki robota
        object.Update(endEffectorTransform);
        UpdateLightValues(shader, lights[0]);   //akrualizujemy światło

        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            Vector2 mouseDelta = GetMouseDelta();
            camAngleX += mouseDelta.x * 0.003f;
            camAngleY -= mouseDelta.y * 0.003f;

            // Ograniczenie pionowego kąta (aby nie przeskoczyć przez zenit)
            if (camAngleY < 0.1f) camAngleY = 0.1f;
            if (camAngleY > PI - 0.1f) camAngleY = PI - 0.1f;
        }

        // OPCJONALNY ZOOM SCROLLEM
        camDistance -= GetMouseWheelMove() * 2.0f;
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) DisableCursor();
        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) EnableCursor();
        if (camDistance < 10.0f) camDistance = 10.0f;
        if (camDistance > 300.0f) camDistance = 300.0f;

        // AKTUALIZACJA POZYCJI KAMERY NA PODSTAWIE KĄTÓW I ODLEGŁOŚCI
        camera.position.x = camTarget.x + camDistance * sinf(camAngleY) * cosf(camAngleX);
        camera.position.y = camTarget.y + camDistance * cosf(camAngleY);
        camera.position.z = camTarget.z + camDistance * sinf(camAngleY) * sinf(camAngleX);
        camera.target = camTarget;
        camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };


        //zaczynamy rysowanie
        BeginDrawing();
            ClearBackground(WHITE); //tło

            BeginMode3D(camera);    //zaczynamy rysowanie 3D
                DrawPlane((Vector3){ 0.0f, -24.0f, 0.0f },    // Pozycja środka
                    (Vector2){ 300.0f, 300.0f },        // Rozmiar podłoża
                    LIGHTGRAY);  //kolor podłoża

                float gridY = -23.9f;
                int gridSize = 15;
                float spacing = 10.0f;
                for (int i = -gridSize; i <= gridSize; i++)
                {
                    // Linie wzdłuż osi Z
                    DrawLine3D((Vector3){ i * spacing, gridY, -gridSize * spacing },(Vector3){ i * spacing, gridY,  gridSize * spacing },BLACK);
                    // Linie wzdłuż osi X
                    DrawLine3D((Vector3){ -gridSize * spacing, gridY, i * spacing },(Vector3){  gridSize * spacing, gridY, i * spacing },BLACK);
                }
                BeginShaderMode(shader);    //zaczynamy rysowaniez shaderami
                    robot.Draw();       //rysujemy robota
                    object.Draw();        //rysujemy sześcian //teraz dodałem
                    DrawModel(groundModel, groundPosition, 1.0f, {198, 209, 252}); //rysujemy podłoże
                EndShaderMode();    //kończymy rysowanie z shaderami
            EndMode3D();            // i ryswoanie 3D

        DrawText("KAROL - The BOKSER", 10, 10, 20, WHITE); //Tekst w lewym górnym rogu
        EndDrawing();
    }
    //zwalnianie pamięci
    robot.Unload();
    object.Unload(); // teraz dodałem
    UnloadModel(groundModel);
    UnloadShader(shader);
    CloseWindow();
    return 0;
}
