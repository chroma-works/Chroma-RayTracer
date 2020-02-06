//#include <thirdparty\glad\include\glad\glad.h>
//#include <thirdparty/glfw-3.3/include/GLFW/glfw3.h>

#include <iostream>
#include <ray-tracer/editor/AssetImporter.h>
#include <ray-tracer/editor/Editor.h>
#include <ray-tracer/main/Window.h>
#include <ray-tracer/editor/Logger.h>
#include <ray-tracer/editor/Shader.h>

#include <thirdparty/tinyxml2/tinyxml2.h>
#include <ray-tracer\main\Scene.h>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1440;

const std::string vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const std::string fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";

int main()
{
	Chroma::Logger::Init();
	CH_INFO("Chroma Ray Tracer v.0.1");

	Chroma::Window* window = new  Chroma::Window(SCR_WIDTH, SCR_HEIGHT, "Chroma Ray Tracer");
	//init editor
	Chroma::Editor editor(window);

	// build and compile our shader program
	// ------------------------------------
	// Create and compile our GLSL program from the shaders
	Chroma::Shader* shader = Chroma::Shader::ReadAndBuildShaderFromFile("../../assets/shaders/phong.vert", "../../assets/shaders/phong.frag");

	//Model import
	Chroma::Mesh* r_mesh = Chroma::AssetImporter::LoadMeshFromOBJ("../../assets/models/rabbit.obj");
	Chroma::Material* mat = new Chroma::Material("u_Material",
		glm::vec3({ 0.8f, 0.8f, 0.8f }), glm::vec3({ 0.8f, 0.8f, 0.8f }), glm::vec3({ 1.0f, 1.0f, 1.0f }), 90.0f);
	std::shared_ptr<Chroma::SceneObject> rabbit = std::make_shared<Chroma::SceneObject>(*r_mesh, "rabbit");

	//rabbit->SetTexture(*texture);
	rabbit->SetMaterial(*mat);

	Chroma::Mesh* b_mesh = Chroma::AssetImporter::LoadMeshFromOBJ("../../assets/models/box.obj");
	Chroma::Texture* texture = new Chroma::Texture("../../assets/textures/crate.jpg");
	std::shared_ptr<Chroma::SceneObject> box = std::make_shared<Chroma::SceneObject>(*b_mesh, "box");

	box->SetTexture(*texture);
	box->SetMaterial(*mat);

	std::shared_ptr<Chroma::Scene> scene;

	scene = std::make_shared<Chroma::Scene>("The scene", shader);
	scene->AddSceneObject("rabbit", rabbit);
	scene->AddSceneObject("box", box);

	glEnable(GL_DEPTH_TEST);

	Chroma::Camera* cam = new Chroma::Camera(1.0f * 1280,
		1.0f * 720, 0.1f, 300.0f, 45.0f);
	//OrthographicCamera cam2(-0.8f, 0.8f, -0.9, 0.9, -10, 10);
	cam->SetPos({ -0.0f, 0.0f, 50.0f });
	cam->SetUp({ -0.0f, 1.0f, 0.0f });
	cam->SetGaze(cam->GetPos() + glm::vec3(0.0, 0.0, -1.0f));
	scene->AddCamera("pers-cam", cam);

	std::shared_ptr<Chroma::DirectionalLight> dl = std::make_shared<Chroma::DirectionalLight>(glm::vec3(-30.0f, 0.0f, -40.0f),
		glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));

	scene->AddLight("directional l", dl);

	glm::vec4 dir({ 0.0f, 0.0f, 0.0f, 1.0f });

	rabbit->SetScale({ .4f, .4f, .4f });
	rabbit->SetPosition({ 0.0f, -9.0f, 0.0f });
	rabbit->SetRotation(glm::quat({ glm::radians(-90.0f), glm::radians(90.0f), glm::radians(0.0f) }));

	box->SetScale({ .9f, .9f, .9f });
	box->SetPosition({ 0.0f, 0.0f, 0.0f });
	box->RotateAngleAxis(glm::radians(180.0), glm::vec3(0.0, 0.0, 1.0));

	tinyxml2::XMLDocument doc;
	doc.LoadFile("../../assets/scenes/simple.xml");

	/*tinyxml2::XMLText* textNode = doc.FirstChildElement("Scene")->FirstChildElement("Cameras")->FirstChild()->FirstChildElement("Position")->FirstChild()->ToText();
	CH_TRACE(textNode->Value());*/

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	// render loop
	// -----------
	while (!window->ShouldClose())
	{
		// input

		// render
		// ------
		//cam->SetPos(cam->GetPos() + glm::vec3(0.0, 0.0, .1f));
		editor.OnUpdate();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene->Render();

	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}