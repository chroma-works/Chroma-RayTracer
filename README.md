#  Chroma Ray Tracer <img align="left" src= "resources/logo_solo.png" height="40">
## Development Blog 
### Author:Alper Şahıstan([STLKRv1](https://github.com/STLKRv1))  

[Accidental Art Page](ACCIDENTALART.html)

<p align="center">
<img src= "resources/logo_w.png" height="100">
 </p>

## Introduction  
This repository is meant to contain a simple implementation of the ray tracing algorithm. Hopefully will surpass its humble conception. This page will also be used as a blog to update on the development process. Developed for CENG795 course at METU(2019-2020 Spring Semester).

<p align="center">
<img src= "resources/editorprev.png" height="400">
 </p>

Here is a full preview of Chroma-Ray Tracer enviroment

## Build instructions for Windows  
### Requires:
* Visual Studio(VS) 17+
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
  
After some fiddling with the code I have found the errors. One of the particular errors I have encountered was causing occluded objects to be rendered as if the objects that occlude them were "transparent"(see **Figure 5**). I quickly realised one must take the smallest t value when considering ray hits at ray direction t. After solving this problem, I was able to implement the ray tracer without shadows. **Figure 6** displays spheres scene without shadows. I have implemented Möller-Trumbore's method for ray-triangle intersections which is a efficient implementation of Cramer's rule[[2]](#2). The introductary article and the intersection methods on the scratch a pixel website was also utilized[[3]](#3). For the sphere intersection I implemented a simple "discriminant-based" method.  

<img src= "resources/deptproblem.png" height="300">  
**Figure 5:** Failed render of the Cornell box due to dept problem.   


<img src= "resources/spheres_no_shadow.png" height="300">  
**Figure 6:** Shadowless render of spheres scene. 

## Week 3  
To begin with shadows, I have implemented a loop that goes over all the scene objects to check if they occlude the light or not. I have used the same intersection methods in the **week 2** to check if the rays cast from intersection point to light position intersects with another scene object. However, this part deemed it self to be very challenging since there were small intricacies.  
First of those intricacies was tracking the t value in the ray equation. This was important because our shadow rays may reach to the light and keep going until they intersect an object behind the light where we mark this ray as shadowed although it is not. **Figure 7** illustrates this problem. And **Figure 7** displays the render failure due to this error.  
<img src= "resources/shdwraymiss.png" width="300"><img src= "resources/shdwraymissc.png" width="300">  
**Figure 7:** On left object is shadowed due to extra intersection, on right correct calculation is shown.  

<img src= "resources/cornellbox_wrong_t.png" height="300">  
**Figure 8:** Failed render of the Cornell box due to incorrect shadowing.  

Another problem I have encountered was the mistake in the Scratch a pixel's implementation of the Möller-Trumbore method. This implementation did not returned false when t in the ray equation is negative. So a ray that has an object behind it's origin returned true although it should not have. Of course this was not a problem for the camera rays since they did not encounter anythşng behind them yet this was causing problems when we triend to calculate shadow rays. Thats why this error went unnoticed until now(and possibly by the authors of the article [[3]](#3) ).  **Figure 9** Demonstrates the render failure due to this problem.
I have tweeted about the problem if one deems this subject interesting he can follow the link: [twitter stlkr_v1's tweet](https://twitter.com/stlkr_v1/status/1229527293351124992)  
  
<img src= "resources/scienceTreefail.png" height="300">  
**Figure 9:** Render failure at the science tree scene due to negative t values for shadow rays.  

To conclude interms of HW1, here are my final renders of all the scenes provided:  
<img src= "resources/simple.png" height="400"> <img src= "resources/spheres.png" height="400">   
<img src= "resources/cornellbox.png" height="400"> <img src= "resources/bunny.png" height="400">  
**Figure 10:** Final renders for HW1. 
  
<img src= "resources/scienceTree.png" width = "804">  
**Figure 11:** Final render of the ODTU "science tree" and logo. ***This can be considered as treason among "Bilkenters".***  

This week I have managed to multi-thread the ray-tracer.**Table 1** shows some statistics including thread counts. 

**Table 1:** Scene and render statistics.  
  
| Scene Name    |# of Triangles & Spheres | Resolution    | # of Treads   |Render time(s) |  
| ------------- | ----------------------- | ------------- | ------------- | ------------- |  
| Simple        | 5,1                     | 800x800       |  8            | 0.1071        |  
| Spheres       | 6,5                     | 1440x720      |  8            | 0.1644        |  
| Cornell Box   | 10,2                    | 800 x800      |  8            | 0.5071        |  
| Bunny         | 4968,0                  | 512x512       |  8            | 13.1100       |  
| Science Tree  | 2240,0                  | 1440x720      |  8            | 28.8958       |   
  
  
## Week 4  
For this week, some acceleration structures have been implemented. To begin with this task, the interface for the acceleration structure has been implemented while maintaining the old classes. After that some experimentations with bounding boxes are made. After expriencing their gains and limitations. Bounding Volume Hierarchy(BVH) structure was implemented. For this task I have followed the Scratchapixel's tutorial about acceleration structures[[4]](#4).  
As expected, during this process I have encountered some bugs;
* The tutorial did not consider spheres as seprate objects with different intersection geometry.  
  * Solved sampling points over each sphere's surface to feed into BVH bounds.  
* Nearly all meshes provided in the hw1 were "thin meshes" meaning all of their vertices lay in the same plane. This caused a major problem for BVH since properly bounding them and finding their intersection from both sides of their faces using those bounds was difficult(see **Figure 12** for the render fail).  
  * Solved by bounding meshes with slightly larger boxes.
I have also made adjustments to my mesh intersection methods since they rendered with minor backface-clipping issiues.  

<img src= "resources/thin_mesh_prob1.png" width = "300"> <img src= "resources/thin_mesh_prob2.png" width = "300">   
**Figure 12:** Cornell box scene cannot be traced with tight BVH bounds since it contains "thin meshes".  

**Table 2** displays new render statistics with BVH using the scenes from hw1.  

**Table 2:** Scene and render statistics with BVH acceleration 
  
| Scene Name    |# of Triangles & Spheres | Resolution    | # of Treads   |Render time(s) |  
| ------------- | ----------------------- | ------------- | ------------- | ------------- |  
| Simple        | 5,1                     | 800x800       |  8            | 0.0328        |  
| Spheres       | 6,5                     | 1440x720      |  8            | 0.1714        |  
| Cornell Box   | 10,2                    | 800 x800      |  8            | 0.2538        |  
| Bunny         | 4968,0                  | 512x512       |  8            | 5.8057        |  
| Science Tree  | 2240,0                  | 1440x720      |  8            | 2.2952        |   
  
  

## Week 5
Implementation for a (recursive) path tracer has been completed. Now transparent and reflective objects can be rendered physically accurate. To calculate these we have considered 2 types of materials; conductors and dielectrics(and also mirrors). The programming the ray calculations for the conductors (and mirrors) were fairly easy since they only reflect the light that hits their surface. However dielectrics pose a challenge since they have many cases to handle along side with reflecions such as refractions, total internal reflections and light absorptions.  
To find the amount of light emmited from the dielectric and conductor materials we use Fresnel equations along side with Snell equations to find the refraction angles[[5]](#5). We also use Beer's Law to find light absorptions when passing through a medium.  
Despide all my efforts there remains to be a small refraction error when we examine the dielectic materials. **Figure 13** shows the error in the science tree scene. The problem was due to applying shading inside a medium. This is not physically correct since we cannot draw a straight line from the internal reflection/refraction point to the light source. After disabling the shading I was able to get the correct render.  

<img src= "resources/scienceTree_failed_glass.png" width="430"> <img src= "resources/scienceTree_correct_glass.png" width="430">  
**Figure 13:** Left shows the failed render with missing dark refractions; right shows the correct render(courtesy of Assoc. Prof. Oğuz Akyüz).  

As it turns out my previous implementation og BVH was not dividing objects in them selves but it was just dividing the scene until the bounding boxes of the objects were reached. So I had to decouple and scrap the **whole acceleration structure hierarchy**. Let me tell you this: "It was **NOT** easy!". But I made it. I was able to imlement a BVH very similar to PBRT version. So I am using Surface Area Heuristic(SAH) to divide the scene(and objects in it!).Additionally, I was able to adapt other heuristics from PBRT[[5]](#5). After implementation all of my provided scenes were rendering faster and correctly.Except the Spheres scene that scene where I had the self collision problem of some rays. Yet this was an odd error since shadow achne was only appering on the right hemisphere of the "sphere" objects. And it was only for the **shadow ray of a diffuse** sphere(not a dielectric or conductor one).**Figure 14** depicts the render fail. I was able to solve this by applying a lower bound threshold for t on BVH bounding box intersections.  

<img src= "resources/half_spheres.png" width="430">   
**Figure 14:** Incorrect shadows due to shadow rays colliding with spheres they are cast from.  
  
To conclude interms of HW2, **Figure 15** displays my final renders of all the scenes provided and **Table 3** shows the render statistics for the scenes using SAH BVH:  
<img src= "resources/cornellbox_recursive.png" height="400"> <img src= "resources/spheres_mirror.png" height="400">   
<img src= "resources/chinese_dragon.png" height="400"> <img src= "resources/scienceTree_correct_glass.png" height="480">  
<img src= "resources/other_dragon.png" height="480">   
**Figure 15:** Final renders for HW1. 

**Table 3:** Scene and render statistics using SAH BVH acceleration
  
| Scene Name    |# of Triangles & Spheres | Resolution    | # of Treads   |Render time(s) | Recursion Depth| BVH size(MB) | # of BVH nodes |  
| ------------- | ----------------------- | ------------- | ------------- | ------------- |----------------|--------------|-------------------|  
| Cornell Box   | 10,2                    | 800x800       |  8            | 0.2201        | 6              | 0.0004       | 13 |  
| Spheres       | 6,5                     |  720x720      |  8            | 0.1328        | 6              | 0.0003       | 11 |  
| Chinese(Stanford) Dragon| 871414,2      | 800 x800      |  8            | 0.1711        | 6              | 52.5348      | 1721461 |  
| Science Tree  | 2240,0                  | 1440x720      |  8            | 0.9363        | 6              | 0.1352       | 4431 |  
| Other Dragon  | 1848604,0               | 800x480       |  8            | 1.2212        | 6              | 112.8229     | 3696981 |  

I have also mixed and matched some of the scene vertex data, lighting, material, etc. to make some new walpapers  

<img src= "resources/glass_dragon.png">
<img src= "resources/dragon.png">
**Figure 16:** Additional renders of "Glass Dragon" and "Stanford Dragon in Cornell box" scenes.
 
  
## References
<a id="1">[1]</a>
Chroma-Works, “chroma-works/Chroma-Engine,” GitHub, 15-Aug-2019. [Online]. Available: https://github.com/chroma-works/Chroma-Engine. [Accessed: 07-Feb-2020].  
<a id="2">[2]</a>
T. Möller and B. Trumbore, “Fast, Minimum Storage Ray-Triangle Intersection,” Journal of Graphics Tools, vol. 2, no. 1, pp. 21–28, 1997.  
<a id="3">[3]</a>
Scratchapixel, Ray Tracing: Rendering a Triangle (Möller-Trumbore algorithm), 15-Aug-2014. [Online]. Available: https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection. [Accessed: 18-Feb-2020].  

<a id="4">[4]</a>
Scratchapixel, “Introduction to Acceleration Structures,” Scratchapixel, 08-Oct-2015. [Online]. Available: https://www.scratchapixel.com/lessons/advanced-rendering/introduction-acceleration-structure/introduction. [Accessed: 29-Feb-2020].

<a id="5">[5]</a>
M. Pharr, W. Jakob , and G. Humphreys , “Physically Based Rendering,” Physically Based Rendering, 2017.
