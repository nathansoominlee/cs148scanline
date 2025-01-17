#include "assignment3/Assignment3.h"
#include "common/core.h" // <-- haha.
#include "common/Utility/Mesh/Simple/PrimitiveCreator.h"
#include "common/Utility/Mesh/Loading/MeshLoader.h"
#include "common/Utility/Texture/TextureLoader.h"

//#include "instructor/Light/EpicLightingProperties.h"
//#include "instructor/Light/DirectionalLight.h"
//#include "instructor/Light/HemisphereLight.h"
//#include "instructor/Shaders/EpicShader.h"
#include <cmath>

Assignment3::Assignment3(std::shared_ptr<class Scene> inputScene, std::shared_ptr<class Camera> inputCamera):
    Application(std::move(inputScene), std::move(inputCamera))
{
}

std::unique_ptr<Application> Assignment3::CreateApplication(std::shared_ptr<class Scene> scene, std::shared_ptr<class Camera> camera)
{
    return make_unique<Assignment3>(std::move(scene), std::move(camera));
}

std::shared_ptr<class Camera> Assignment3::CreateCamera()
{
    // Specify any old aspect ratio for now, we'll update it later once the window gets made!
    // Read more about Field of View: http://rg3.name/201210281829.html!
    // Note that our field of view is the VERTICAL field of view (in degrees).
    return std::make_shared<PerspectiveCamera>(60.f, 1280.f / 720.f);
}

glm::vec2 Assignment3::GetWindowSize() const
{
    return glm::vec2(1280.f, 720.f);
}

void Assignment3::SetupScene()
{
    SetupCamera();
    SetupDummy();
}

//Adjust the camera for viewing as in Maya
void Assignment3::SetupCamera()
{
    
    PerspectiveCamera* pcamera = static_cast<PerspectiveCamera*>(camera.get());
  
    // Set camera field of view and clipping planes
    pcamera->SetFOV(35.9f);
    pcamera->SetZNear(0.1f);
    pcamera->SetZFar(1000.f);
    
    // Set camera position and rotation
    camera->SetPosition(glm::vec3(-7.339f, 135.659f, 429.348f));
    camera->Rotate(glm::vec3(1.f, 0.f, 0.f), -4.985f * PI / 180.f);
    camera->Rotate(glm::vec3(0.f, 1.f, 0.f), -0.942f* PI / 180.f);
    camera->Rotate(glm::vec3(0.f, 0.f, 1.f), 0.f * PI / 180.f);
    
}

void Assignment3::HandleInput(SDL_Keysym key, Uint32 state, Uint8 repeat, double timestamp, double deltaTime)
{   
    if (state != SDL_KEYDOWN) {
        return;
    }

    switch (key.sym) {
    case SDLK_1:
        if (!repeat && state == SDL_KEYDOWN) {
            SetupDummy();
        }
        break;
    case SDLK_2:
        if (!repeat && state == SDL_KEYDOWN) {
            SetupEpic();
        }
        break;
    case SDLK_p:
        AddPLight();
        break;
    case SDLK_d:
        AddDLight();
        break;
    case SDLK_h:
        AddHLight();
        break;
    case SDLK_c:
        scene->ClearLights();
        break;
    case SDLK_UP:
        camera->Rotate(glm::vec3(camera->GetRightDirection()), 0.1f);
        break;
    case SDLK_DOWN:
        camera->Rotate(glm::vec3(camera->GetRightDirection()), -0.1f);
        break;
    case SDLK_RIGHT:
        camera->Rotate(glm::vec3(SceneObject::GetWorldUp()), -0.1f);
        break;
    case SDLK_LEFT:
        camera->Rotate(glm::vec3(SceneObject::GetWorldUp()), 0.1f);
        break;
    case SDLK_SPACE:
        camera->Translate(glm::vec3(camera->GetUpDirection() * 0.3f));
        break;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
        camera->Translate(glm::vec3(camera->GetUpDirection() * -0.3f));
        break;
    default:
        Application::HandleInput(key, state, repeat, timestamp, deltaTime);
        break;
    }
}

void Assignment3::HandleWindowResize(float x, float y)
{
    Application::HandleWindowResize(x, y);
    std::static_pointer_cast<PerspectiveCamera>(camera)->SetAspectRatio(x / y);
}





void Assignment3::SetupDummy()
{
    scene->ClearScene();
    
    std::unordered_map<GLenum, std::string> shaderSpec = {
        { GL_VERTEX_SHADER, "brdf/epicshader/frag/noSubroutine/epicshader.vert" },
        { GL_FRAGMENT_SHADER, "brdf/epicshader/frag/noSubroutine/epicshader.frag"}
    };
    
    std::shared_ptr<EpicShader> shader = std::make_shared<EpicShader>(shaderSpec, GL_FRAGMENT_SHADER);
    shader->SetMetallic(0.5f);
    shader->SetRoughness(0.f);
    shader->SetSpecular(0.5f);
    
    std::vector<std::shared_ptr<RenderingObject>> meshTemplate = MeshLoader::LoadMesh(shader, "dummy/Model/dummy.obj");
    if (meshTemplate.empty()) {
        std::cerr << "ERROR: Failed to load the model. Check your paths." << std::endl;
        return;
    }       

    std::shared_ptr<class SceneObject> sceneObject = std::make_shared<SceneObject>(meshTemplate);
    scene->AddSceneObject(sceneObject);
    
    AddPLight();
    
}

void Assignment3::AddPLight()
{
    // turn on point light.
    std::unique_ptr<LightProperties> lightProperties = make_unique<LightProperties>();
    lightProperties->color = glm::vec4(2.f, 2.f, 2.f, 1.f);
    lightProperties->radius = 1000.f;
    
    pointLight = std::make_shared<Light>(std::move(lightProperties));
    pointLight->SetPosition(glm::vec3(0.f, 0.f, 10.f));
    scene->AddLight(pointLight);
    pLightIsOn = true;
}

void Assignment3::AddDLight()
{
    // add a directional light which has a direction instead of position
    std::unique_ptr<LightProperties> lightProperties = make_unique<LightProperties>();
    lightProperties = EpicShader::CreateLightProperties();
    lightProperties->color = glm::vec4(2.f, 2.f, 2.f, 1.f);
    lightProperties->direction = glm::vec4(0.f, -10.f, 0.f, 1.f); // Sunlight shines down (in the -y direction)
    
    sunLight = std::make_shared<Light>(std::move(lightProperties), Light::LightType::DIRECTIONAL);
    scene->AddLight(sunLight);
}

void Assignment3::AddHLight()
{
    // add a hemispherical light which has a sky and ground color
    std::unique_ptr<LightProperties> lightProperties = make_unique<LightProperties>();
    lightProperties = EpicShader::CreateLightProperties();
    lightProperties->groundColor = glm::vec3(0.f, 3.f, 1.f); // Green
    lightProperties->skyColor = glm::vec3(1.f, 1.f, 3.f);    // Blue
    lightProperties->radius = 1000.f;
    
    hemisphereLight = std::make_shared<Light>(std::move(lightProperties), Light::LightType::HEMISPHERE);
    scene->AddLight(hemisphereLight);
}


void Assignment3::SetupEpic()
{
    scene->ClearScene();
    std::unordered_map<GLenum, std::string> shaderSpec = {
        { GL_VERTEX_SHADER, "brdf/epicshader/frag/noSubroutine/epicshader.vert" },
        { GL_FRAGMENT_SHADER, "brdf/epicshader/frag/noSubroutine/epicshader.frag"}
    };
    
    std::shared_ptr<EpicShader> shader = std::make_shared<EpicShader>(shaderSpec, GL_FRAGMENT_SHADER);
    shader->SetMetallic(0.5f);
    shader->SetRoughness(0.f);
    shader->SetSpecular(0.5f);
    
    std::shared_ptr<EpicShader> groundShader = std::make_shared<EpicShader>(shaderSpec, GL_FRAGMENT_SHADER);
    groundShader->SetMetallic(0.f);
    groundShader->SetRoughness(0.f);
    groundShader->SetSpecular(0.f);
    
    AddPLight();
    AddDLight();
    AddHLight();
    
    GenericSetupExample(shader, groundShader);
    
}

void Assignment3::GenericSetupExample(std::shared_ptr<ShaderProgram> shader, std::shared_ptr<ShaderProgram> groundShader)
{
    std::shared_ptr<RenderingObject> sphereTemplate = PrimitiveCreator::CreateIcoSphere(shader, 5.f, 4);

    // Give a R/G/B color to each vertex to visualize the sphere.
    auto totalVertices = sphereTemplate->GetTotalVertices();

    std::unique_ptr<RenderingObject::ColorArray> vertexColors = make_unique<RenderingObject::ColorArray>();
    vertexColors->reserve(totalVertices);

    for (decltype(totalVertices) i = 0; i < totalVertices; ++i) {
        vertexColors->emplace_back(1.f, 1.f, 1.0f, 1.f);
    }
    sphereTemplate->SetVertexColors(std::move(vertexColors));

    sphereDance.clear();
    // 10x10 grid of spheres.
    for (int x = 0; x < 10; ++x)  {
        for (int y = 0; y < 10; ++y) {
            std::shared_ptr<class SceneObject> sceneObject = std::make_shared<SceneObject>(sphereTemplate);
            sceneObject->SetPosition(glm::vec3(x * 12.f, 0.f, y * 12.f));
            scene->AddSceneObject(sceneObject);
            sphereDance.push_back(sceneObject);
        }
    }

    std::shared_ptr<RenderingObject> plane = PrimitiveCreator::CreatePlane(groundShader);
    std::shared_ptr<class SceneObject> groundObject = std::make_shared<SceneObject>(plane);
    groundObject->MultScale(150.f);
    groundObject->SetPosition(glm::vec3(-10.f, 0.f, -10.f));
    groundObject->Translate(glm::vec3(SceneObject::GetWorldUp() * -5.f));

    std::unique_ptr<RenderingObject::ColorArray> groundColor = make_unique<RenderingObject::ColorArray>();
    groundColor->reserve(4);

    for (int i = 0; i < 4; ++i) {
        groundColor->emplace_back(0.89, 0.349f, 0.f, 1.f);
    }
    plane->SetVertexColors(std::move(groundColor));

    scene->AddSceneObject(groundObject);
}

void Assignment3::Tick(double deltaTime)
{
    if (sunLight) {
        sunLight->Rotate(glm::vec3(SceneObject::GetWorldRight()), PI * deltaTime / -4.f);
    }

    static float elapsedTime = 0.f;
    elapsedTime += deltaTime;

    for (size_t i = 0; i < sphereDance.size(); ++i) {
        glm::vec4 og = sphereDance[i]->GetPosition();
        sphereDance[i]->SetPosition(glm::vec3(og.x, std::abs(std::sin(elapsedTime + (float)i)) * 6.f, og.z));
    }
}











/* FOR REFERENCE
 void Assignment3::SetupExample1()
 {
 scene->ClearScene();
 #ifndef DISABLE_OPENGL_SUBROUTINES
 std::unordered_map<GLenum, std::string> shaderSpec = {
 { GL_VERTEX_SHADER, "brdf/blinnphong/frag/blinnphong.vert" },
 { GL_FRAGMENT_SHADER, "brdf/blinnphong/frag/blinnphong.frag" }
 };
 #else
 std::unordered_map<GLenum, std::string> shaderSpec = {
 { GL_VERTEX_SHADER, "brdf/blinnphong/frag/noSubroutine/blinnphong.vert" },
 { GL_FRAGMENT_SHADER, "brdf/blinnphong/frag/noSubroutine/blinnphong.frag"}
 };
 #endif
 std::shared_ptr<BlinnPhongShader> shader = std::make_shared<BlinnPhongShader>(shaderSpec, GL_FRAGMENT_SHADER);
 shader->SetDiffuse(glm::vec4(0.8f, 0.8f, 0.8f, 1.f));
 shader->SetSpecular(glm::vec4(1.f, 1.f, 1.f, 1.f), 40.f);
 
 std::shared_ptr<BlinnPhongShader> groundShader = std::make_shared<BlinnPhongShader>(shaderSpec, GL_FRAGMENT_SHADER);
 shader->SetDiffuse(glm::vec4(0.8f, 0.8f, 0.8f, 1.f));
 
 std::unique_ptr<BlinnPhongLightProperties> lightProperties = BlinnPhongShader::CreateLightProperties();
 lightProperties->diffuseColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
 lightProperties->specularColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
 
 pointLight = std::make_shared<Light>(std::move(lightProperties));
 pointLight->SetPosition(glm::vec3(10.f, 10.f, 10.f));
 scene->AddLight(pointLight);
 
 GenericSetupExample(shader, groundShader);
 
 }
 
 void Assignment3::SetupExample2() {
 //    scene->ClearScene();
 //    std::unordered_map<GLenum, std::string> shaderSpec = {
 //        { GL_VERTEX_SHADER, "instructor/epic/epic.vert" },
 //        { GL_FRAGMENT_SHADER, "instructor/epic/epic.frag" }
 //    };
 //    std::shared_ptr<EpicShader> shader = std::make_shared<EpicShader>(shaderSpec, GL_FRAGMENT_SHADER);
 //    shader->SetMetallic(0.6f);
 //    shader->SetSpecular(0.4f);
 //    shader->SetRoughness(0.5f);
 //
 //    std::shared_ptr<EpicShader> groundShader = std::make_shared<EpicShader>(shaderSpec, GL_FRAGMENT_SHADER);
 //    groundShader->SetMetallic(0.f);
 //    groundShader->SetSpecular(0.f);
 //    groundShader->SetRoughness(0.f);
 //
 //    std::unique_ptr<EpicLightingProperties> lightProperties = EpicShader::CreateLightProperties();
 //    lightProperties->lightColor = glm::vec4(2.f, 2.f, 2.f, 1.f);
 //
 //    sunLight = std::make_shared<DirectionalLight>(std::move(lightProperties));
 //    sunLight->Rotate(glm::vec3(SceneObject::GetWorldRight()), PI / -4.f);
 //    sunLight->Rotate(glm::vec3(SceneObject::GetWorldUp()), PI / 4.f);
 //    scene->AddLight(sunLight);
 //
 //    lightProperties = EpicShader::CreateLightProperties();
 //    lightProperties->lightColor = glm::vec4(glm::vec3(0.529f, 0.808f, 0.98f) * 3.f, 1.f);
 //    lightProperties->secondaryColor = glm::vec4(glm::vec3(0.471f, 0.282f, 0.f) * 3.f, 1.f);
 //    hemisphereLight = std::make_shared<HemisphereLight>(std::move(lightProperties));
 //    scene->AddLight(hemisphereLight);
 //
 //    GenericSetupExample(shader, groundShader);
 }
 */
