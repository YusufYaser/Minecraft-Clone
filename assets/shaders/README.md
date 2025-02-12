# Shaders Directory

You can add custom shaders such as `PostProcessing` to override the default game shaders

The directory structure must be like this:
```ruby
ShaderName
  ShaderName.vert
  ShaderName.frag
```
You don't need to have both the vertex and fragment shader at the same time.

Game Shaders: `PostProcessing`, `DefaultShader`, `GUI`, `Skybox`\
You can copy the default shaders from [the source code](/src/Shaders).

You can reload the game shaders using `/ + U`
