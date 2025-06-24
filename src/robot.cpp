#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "rlights.h"
#define RLIGHTS_IMPLEMENTATION
#include "robot.h"
#include "object.h"
#include "record.h"
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
    Light lights[MAX_LIGHTS] = {0, 1, 2};  //tablica świateł, póki co jest jedno, ale może sie pokusimy o różny rodzaj oświetlenia :)
    lights[0] = CreateLight(LIGHT_POINT, LightPos, Vector3Zero(), WHITE, shader); //parametry światła (punktowe)
    lights[1] = CreateLight(LIGHT_POINT, (Vector3){-25.0f, 30.0f, -50.0f}, Vector3Zero(), (Color){128, 128, 40, 255}, shader); // światło punktowe, które świeci z drugiej strony
    lights[2] = CreateLight(LIGHT_POINT, (Vector3){50.0f, 30.0f, -50.0f}, Vector3Zero(), (Color){80, 130, 230, 255}, shader);
    bool yellowLightOn = true;
    bool blueLightOn = true;
    //wczytujemy robota z załadowanym wyżej shaderem
    Robot robot(shader);
    Object object(shader); //przykładowy obiekt do testowania
    object.Initialize(); //inicjalizacja sześcianu
    //Deklaracja recordera
    MovementRecorder recorder;

    //generowanie podłoża
    Mesh groundMesh = GenMeshPlane(200.0f, 200.0f, 1, 1);
    Model groundModel = LoadModelFromMesh(groundMesh);
    Vector3 groundPosition = { 0.0f, 0.0f, 0.0f };
    Matrix endEffectorTransform = MatrixIdentity();


    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // DODAJ OBSŁUGĘ KLAWISZY NAGRYWANIA:
        if (IsKeyPressed(KEY_R)) {
            if (recorder.currentMode == NORMAL_MODE) {
                recorder.StartRecording(robot.jointTransforms);
            } else if (recorder.currentMode == RECORDING_MODE) {
                recorder.StopRecording();
            }
        }
        
        if (IsKeyPressed(KEY_P) && recorder.currentMode == NORMAL_MODE) {
            recorder.StartPlayback();
        }

        if (recorder.currentMode == NORMAL_MODE) {
        robot.HandleInput();
        object.Input();
        robot.Update();  // Normalne update
        object.Update(robot.jointTransforms[6]);
        }
        else if (recorder.currentMode == RECORDING_MODE) {
            robot.HandleInput();  // Pozwól na ruch podczas nagrywania
            object.Input();
            robot.Update();
            object.Update(robot.jointTransforms[6]);
            recorder.Update(robot.jointTransforms, object.grab);
        }
        else if (recorder.currentMode == PLAYBACK_MODE) {
            // Odtwarzaj sekwencję
            Matrix* currentFrame = recorder.GetCurrentPlaybackFrame();
            if (currentFrame) {
                robot.SetFromTransforms(currentFrame);
                object.grab = recorder.GetCurrentGrabState();
                object.Update(currentFrame[6]);
            }
            recorder.UpdatePlayback();  // DODAJ - aktualizuj indeks odtwarzania
        }

        
        //To było wcześniej
        //robot.HandleInput();    //klawisze do sterowania
        //object.Input(); //klawisz do chwytania sześcianu
        //robot.Update();     //aktualizujemy pozycje i obrot robota
        //endEffectorTransform = robot.jointTransforms[6]; //aktualizujemy transformację końcówki robota
        //object.Update(endEffectorTransform);
        UpdateLightValues(shader, lights[0]);   //akrualizujemy światło
        UpdateLightValues(shader, lights[1]);   //akrualizujemy światło
        UpdateLightValues(shader, lights[2]);   //akrualizujemy światło

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

                float gridY = -23.8f;
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
                    if (IsKeyPressed(KEY_B)) {
                        blueLightOn = !blueLightOn;
                        if (blueLightOn) {
                            lights[2].color = (Color){80, 130, 230, 255}; // włącz
                        } else {
                            lights[2].color = (Color){0, 0, 0, 0}; // wyłącz
                        }
                    }
                    
                    if (IsKeyPressed(KEY_N)) {
                        yellowLightOn = !yellowLightOn;
                        if (yellowLightOn) {
                            lights[1].color = (Color){128, 128, 40, 255}; // włącz
                        } else {
                            lights[1].color = (Color){0, 0, 0, 0}; // wyłącz
                        }
                    }
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