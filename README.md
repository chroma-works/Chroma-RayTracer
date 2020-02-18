#  Chroma Ray Tracer <img align="left" src= "resources/logo_solo.png" height="40">
## Development Blog 
### Author:Alper Şahıstan([STLKRv1](https://github.com/STLKRv1)) 

<p align="center">
<img src= "resources/logo_w.png" height="100">
 </p>

## Introduction  
This repository is meant to contain a simple implementation of the ray tracing algorithm. Hopefully will surpass its humble conception. This page will also be used as a blog to update on the development process. Developed for CENG795 course at METU(2019-2020 Spring Semester).

## Build instructions for Windows  
### Requires:
* Visual Stuido(VS) 17+
* CMake 3.15.0+
### Instructions
After downloading and unzipping the repo open Cmake and fill the fields:  
**Where is the source:** unzipped/folders/directory/Chroma-RayTracer  
**Where to build binaries:** unzipped/folders/directory/Chroma-RayTracer/build  
then click configure where you will set your Visual studio version as the **"Generator for this project"*** and *x64* as **"Optional platform generator"**. Also select **"Use default native compilers"** option if it is not already selected.  
**Configure** then **Generate**.  
Then open the chroma-ray-tracer.sln file generated using VS located in the ./Chroma-RayTracer/build folder.  
Finally **Build** solution using VS.  

**Note:** After building the project, if it fails to run due to missing .exe file try setting the location of the .exe file by:  
Solution explorer -> chroma-ray-tracer -> (right click ->) properties -> General -> Output Directory -> 
* if Configuration: Debug -> unzipped/folders/directory/Chroma-RayTracer/bin/Debug  
* if Configuration: Release -> unzipped/folders/directory/Chroma-RayTracer/bin/Release

## Week 1
Although I have not made any advancements regarding "Ray-tracing" I have retrofitted my old classes from my old renderer[[1]](#1) to build a simple editor. This editor can;
* Load .obj files
* Render multiple lights of types:
  * Point lights
  * Directional lights
  * Spot lights
* Support textures
* Allow basic navigation/editing over the scene
* Uses Phong Shading  
Please keep in mind that footages given below are NOT ray traced in any shape or form. They are rendered using OpenGL3.  
Here is a video:  
<blockquote class="twitter-tweet"><p lang="en" dir="ltr">Recently I started developing my RayTracer. Although I have not implemented any sort of RayTracer(yet).I retrofitted my old renderer classes and combined them with a custom ImGui UI to create a simple editor. Here is the result. I MAKE THE BUNNY DISAPPEAR in the end.MAGIC!?🎩🐇 <a href="https://t.co/SqCnVzLw9Z">pic.twitter.com/SqCnVzLw9Z</a></p>&mdash; Alper Ş (@stlkr_v1) <a href="https://twitter.com/stlkr_v1/status/1225586675490971648?ref_src=twsrc%5Etfw">February 7, 2020</a></blockquote>  
<img src= "resources/ui.PNG" height="270">    <img src= "resources/rf.PNG" width="396" >  
**Figure 1:** Snapshot from Scene editor UI  &  Snapshot from Ray tracer window(since it uses GPU texture memory when left uninitialized it fetches some random texture).  
  

 

<img src= "resources/editor.PNG" height="260">  
**Figure 2:** Example render of a Utah teapot and a crate


## Week 2
Asset importer for the scene files has been completed. It simply parses the xml file to construct the scene specified by it. Figure 3 displays the scene called "<a href="assets/scenes/simple.xml">simple.xml</a>"(Provided by course instructor) as rendered in the Raster based rendering pipeline(NOT ray traced).

<img src= "resources/scene_import.gif">  
**Figure 3:** Raster based render of the scene from editor.  

Finally, everything is set. I have started working on the Ray Tracer. Some attemps are made. Sphere-Ray intersection has been implemented and seems to be working. However rays are not allined properly. **Figure 4** includes a screenshot of the sphere in the scene. It should be noted that Ray Tracer currently just casts rays and paints the pixels if they hit an object. Nothing fancy yet.

<img src= "resources/fail.png">  
**Figure 4:** 300x300 image of a ray traced sphere. Failed to properly set ray directions to the near plane. No lighting calculations are made. Just hit tests.   
  
After some fiddeling with the code I have found the errors. And I was able to implement the ray tracer without shadows. **Figure 5** displays spheres scene without shadows. I have implemented Möller-Trumbore's method for ray-triangle intersections which is a efficient implementation of Cramer's rule[[2]](#2). The introductary article and the intersection methods on the scratch a pixel website was also utilized[[3]](#3).  

<img src= "resources/spheres_no_shadow.png" height="300">
**Figure 5:** Shadowless render of spheres scene.  

## References
<a id="1">[1]</a>
Chroma-Works, “chroma-works/Chroma-Engine,” GitHub, 15-Aug-2019. [Online]. Available: https://github.com/chroma-works/Chroma-Engine. [Accessed: 07-Feb-2020].  
<a id="2">[2]</a>
T. Möller and B. Trumbore, “Fast, Minimum Storage Ray-Triangle Intersection,” Journal of Graphics Tools, vol. 2, no. 1, pp. 21–28, 1997. <a id="3">[3]</a>
Scratchapixel, Ray Tracing: Rendering a Triangle (Möller-Trumbore algorithm), 15-Aug-2014. [Online]. Available: https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection. [Accessed: 18-Feb-2020].
