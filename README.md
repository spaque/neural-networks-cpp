# neural-networks-cpp
Neural networks applied to a racing game

![Alt text](/screenshot.png?raw=true)

This was my graduate project. The aim was to apply neural networks to a physically realistic 3D racing game. Implemented entirely in C++, it uses Ogre3D for rendering, Havok for physics simulation and features like multithreading and a custom [memory allocator](https://github.com/ned14/nedmalloc).

Cars are controlled using a multi-layer feed-forward neural network, which was trained using a dataset collected from a human controlled car. See PFC.pdf for design and implementation details.

# Demo

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/q_TgZO7JJ4Q/0.jpg)](https://www.youtube.com/watch?v=q_TgZO7JJ4Q)
