//#include <thirdparty\glad\include\glad\glad.h>
//#include <thirdparty/glfw-3.3/include/GLFW/glfw3.h>

//#define _CRTDBG_MAP_ALLOC  
//#include <stdlib.h>  
//#include <crtdbg.h>  

#include <iostream>
#include <ray-tracer/editor/AssetImporter.h>
#include <ray-tracer/editor/Editor.h>
#include <ray-tracer/main/Window.h>
#include <ray-tracer/editor/Logger.h>
#include <ray-tracer/editor/Shader.h>

#include <ray-tracer\main\Scene.h>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1440;

int main()
{
	Chroma::Logger::Init();
	CH_INFO("Chroma Ray Tracer v.1.9");

	Chroma::Window window = Chroma::Window(SCR_WIDTH, SCR_HEIGHT, "Chroma Ray Tracer");
	Chroma::RayTracer* rt = new Chroma::RayTracer();

	// build and compile our shader program
	// ------------------------------------
	// Create and compile our GLSL program from the shaders
	Chroma::Shader* shader = Chroma::Shader::ReadAndBuildShaderFromFile("../../assets/shaders/phong.vert", "../../assets/shaders/phong.frag");

	std::shared_ptr<Chroma::Scene> scene;
	scene = std::make_shared<Chroma::Scene>(*(Chroma::AssetImporter::LoadSceneFromXML(shader, "../../assets/scenes/hw3/cornellbox_brushed_metal.xml")));
	//init editor
	Chroma::Editor editor(&window, scene.get());

	editor.SetRayTracer(rt);

	//Model import
	/*Chroma::Mesh* r_mesh = Chroma::AssetImporter::LoadMeshFromOBJ("../../assets/models/sport_car.obj");
	Chroma::Texture* text = new Chroma::Texture("../../assets/textures/white.png");
	Chroma::Material* mat = new Chroma::Material("u_Material",
		glm::vec3({ 0.8f, 0.8f, 0.8f }), glm::vec3({ 0.8f, 0.8f, 0.8f }), glm::vec3({ 1.0f, 1.0f, 1.0f }), 90.0f);
	std::shared_ptr<Chroma::SceneObject> teapot = std::make_shared<Chroma::SceneObject>(*r_mesh, "car");*/

	/*rabbit->SetTexture(*texture);
	teapot->SetMaterial(*mat);
	teapot->SetTexture(*text);

	Chroma::Mesh* b_mesh = Chroma::AssetImporter::LoadMeshFromOBJ("../../assets/models/box.obj");
	Chroma::Texture* texture = new Chroma::Texture("../../assets/textures/crate.jpg");
	std::shared_ptr<Chroma::SceneObject> box = std::make_shared<Chroma::SceneObject>(*b_mesh, "box");

	box->SetTexture(*texture);
	box->SetMaterial(*mat);

	 //std::make_shared<Chroma::Scene>("The scene", shader);*/
	/*scene->AddSceneObject("car", teapot);*/
	//scene->AddSceneObject("box", box);

	glEnable(GL_DEPTH_TEST);

	/*Chroma::Camera* cam = new Chroma::Camera(1.0f * 1280,
		1.0f * 720, 0.1f, 300.0f, 45.0f);
	//OrthographicCamera cam2(-0.8f, 0.8f, -0.9, 0.9, -10, 10);
	cam->SetPosition({ -0.0f, 0.0f, 50.0f });
	cam->SetUp({ -0.0f, 1.0f, 0.0f });
	cam->SetGaze(cam->GetPosition() + glm::vec3(0.0, 0.0, -1.0f));
	//scene->AddCamera("pers-cam", cam);

	std::shared_ptr<Chroma::PointLight> pl = std::make_shared<Chroma::PointLight>(glm::vec3(0.0f, 0.0f, 40.0f), glm::vec3(0.1f, 0.1f, 0.1f),
		glm::vec3(0.6f, 0.5f, 0.6f), glm::vec3(1.0f, 1.0f, 1.0f));

	std::shared_ptr<Chroma::DirectionalLight> dl = std::make_shared<Chroma::DirectionalLight>(glm::vec3(-30.0f, 0.0f, -40.0f),
		glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));

	std::shared_ptr<Chroma::SpotLight> sl = std::make_shared<Chroma::SpotLight>(glm::vec3(-20.0f, 0.0f, 40.0f), glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));*/

		//scene->AddLight("point 1", pl);
		//scene->AddLight("spot 1", sl);
		//scene->AddLight("directional 1", dl);

	/*glm::vec4 dir({ 0.0f, 0.0f, 0.0f, 1.0f });

	teapot->SetScale({ 10.0f, 10.0f, 10.0f });
	teapot->SetPosition({ 0.0f, -9.0f, 0.0f });
	teapot->SetRotation(glm::quat({ glm::radians(0.0f), glm::radians(45.0f), glm::radians(0.0f) }));

	/*box->SetScale({ .9f, .9f, .9f });
	box->SetPosition({ 35.0f, 0.0f, 0.0f });
	box->RotateAngleAxis(glm::radians(0.0), glm::vec3(0.0, 0.0, 1.0));*/

	editor.SetScene(scene.get());

	/*tinyxml2::XMLText* textNode = doc.FirstChildElement("Scene")->FirstChildElement("Cameras")->FirstChild()->FirstChildElement("Position")->FirstChild()->ToText();
	CH_TRACE(textNode->Value());*/

	glClearColor(scene->m_sky_color.x, scene->m_sky_color.y, scene->m_sky_color.z, scene->m_sky_color.w);
	// render loop
	// -----------
	while (!window.ShouldClose())
	{
		editor.OnUpdate();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();

	//_CrtDumpMemoryLeaks();
	return 0;
}