# Chroma Ray Tracer Development Blog  
### Author:Alper Şahıstan([STLKRv1](https://github.com/STLKRv1)) 

## Introduction  
This repository is meant to contain a simple implementation of the ray tracing algorithm. Hopefully will surpass its humble conception. This page will also be used as a blog to update on the development process. Developed for CENG795 course at METU(2019-2020 Spring Semester).

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
[Alper Ş @stlkr_v1's tweet](https://twitter.com/stlkr_v1/status/1225586675490971648, "Alper Ş @stlkr_v1's tweet")  

<img src= "resources/ui.PNG" height="270">    <img src= "resources/rf.PNG" width="396" >  
Snapshot from Scene editor UI  &  Snapshot from Ray tracer window(since it uses GPU texture memory when left uninitialized it fetches some random texture).  
  

 

<img src= "resources/editor.PNG" height="260">  
Example render of a utah teapot and create


## References
<a id="1">[1]</a>
Chroma-Works, “chroma-works/Chroma-Engine,” GitHub, 15-Aug-2019. [Online]. Available: https://github.com/chroma-works/Chroma-Engine. [Accessed: 07-Feb-2020].
