#include "Game.h"

Game::Game()
{
    distance_to_player = 9;
    pitch_angle = 15;
    angle_around_player = 0;
    aspectRatio=1;
    speed_angle = 0;

    turnLeft  = false;
    turnRight = false;
    goPlayer = false;
    backPlayer=false;
}


void Game::keyCallback(GLFWwindow* window,int key,int scancode,int action,int mods) {
    if (action==GLFW_PRESS) {
        if (key==GLFW_KEY_A) turnLeft = true;
        if (key==GLFW_KEY_D) turnRight = true;
        if (key==GLFW_KEY_W) goPlayer = true;
        if (key==GLFW_KEY_S) backPlayer = true;
        if (key==GLFW_KEY_V) speed_angle = 2* turn_angle;
    }
    if (action==GLFW_RELEASE) {
        if (key==GLFW_KEY_A) turnLeft = false;
        if (key==GLFW_KEY_D) turnRight = false;
        if (key==GLFW_KEY_W) goPlayer = false;
        if (key==GLFW_KEY_S) backPlayer = false;
         if (key==GLFW_KEY_V) speed_angle = 0;
    }
}


void Game::run(GLFWwindow* window, ShaderProgram *pointer)
{

    sp = pointer;
//Tworzenie obiektów
//----------------------------------------------------------------------------------------------------------------------
    Object track;
    track.loadFromPath("models/track.obj","img/tracktexture.png", vec3(0.0f,-2.0f,0.0f), -90.0f, 0.0f, 0.0f, 2.0f);

    Object cube;
    cube.loadFromPath("models/cube.obj","img/Grass.png", vec3(0.0f,-102.0f,0.0f), -90.0f, 0.0f, 0.0f, 200.0f);

    Car enemy;
    enemy.loadFromPath("models/body.obj","models/chassis.obj","models/headlit.obj","models/license.obj", "models/wheel.obj","img/test.png","img/as3.png","img/s3cos.png","img/license.png","img/texWhee1.png", 0.01,0.05 ,vec3(30.0f,0.0f,0-5.0f), 0.0f,0.0f,0.0f,1.0f);
    enemy.getMarkup()->loadMarkup(0.2);

    Car player;
    player.loadFromPath("models/body.obj","models/chassis.obj","models/headlit.obj","models/license.obj", "models/wheel.obj","img/test.png","img/as3.png","img/s3cos.png","img/license.png","img/texWhee1.png", 0.01,0.05 ,vec3(25.0f,0.0f,10.0f), 0.0f,0.0f,0.0f,1.0f);
    player.getMarkup()->loadMarkup(0.2);


    OBJLoader loader;
    loader.load("models/Tree.obj");

    Object tree[amount_of_trees];
    int i = 0;
    for(i = 0 ; i < amount_of_trees; i++)
    {
        tree[i].loadFromLoader(loader,"img/bricks.png", vec3(0,-2,65-15*i), 0,0,0,2);
    }

//----------------------------------------------------------------------------------------------------------------------

//macierze P i V
//----------------------------------------------------------------------------------------------------------------------
    mat4 P=perspective(50.0f*PI/180.0f, aspectRatio, 0.01f, 500.0f); //Wylicz macierz rzutowania
    mat4 V = mat4(1.0f);
//----------------------------------------------------------------------------------------------------------------------

    setCamera(V, player);
	glfwSetTime(0); //Zeruj timer

//Pêtla g³ówna gry
//----------------------------------------------------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window)) //Tak d³ugo jak okno nie powinno zostaæ zamkniête
	{
        glfwSetTime(0); //Zeruj timer
		drawScene(window, V, P, cube,track, player, tree, enemy); //Wykonaj procedurê rysuj¹c¹
        moving(V, player);                                   //wykonaj procedurê odpowiajaj¹ca za poruszanie graczem oraz kamer¹
        game(cube,track, player, tree, enemy);               //wykonaj procedurê odpowiedzialn¹ za logikê gry
		glfwPollEvents();                                    //Wykonaj procedury callback w zaleznoœci od zdarzeñ jakie zasz³y.
		while(glfwGetTime() < 1/FPS) {}                      //Zapewnij sta³e 60FPS
	}
//----------------------------------------------------------------------------------------------------------------------

}


bool Game::collision(Car &car, Object &object)
{
    if ( pow(car.getBody()->getRadius() - object.getRadius(), 2) <
        pow(car.getBody()->getPosition().x - object.getPosition().x, 2) +
        pow(car.getBody()->getPosition().z - object.getPosition().z, 2)
        )
    {
        if ( pow(car.getBody()->getRadius() + object.getRadius(), 2) >
            pow(car.getBody()->getPosition().x - object.getPosition().x, 2) +
            pow(car.getBody()->getPosition().z - object.getPosition().z, 2)
             )
                {
                    return true;
                }
    }

    return false;
}


float Game::toRadians(float angle)
{
    return angle*3.14f/180.0f;
}


void Game::setCamera(mat4 &V, Car player)
{
    float Camera_y = distance_to_player * sin(toRadians(pitch_angle));
    float Camera_x = distance_to_player * cos(toRadians(pitch_angle));

    float angle = player.getRotation() + toRadians(angle_around_player);
    float offsetX = Camera_x * sin(angle);
    float offsetZ = Camera_x * cos(angle);

    vec3 camera_position;
    camera_position.x = player.getPosition().x - offsetX;
    camera_position.y = player.getPosition().y + Camera_y;
    camera_position.z = player.getPosition().z - offsetZ;

    V = lookAt(
               camera_position,
               player.getPosition(),
               vec3(0, 1, 0 )
              );
}

//Procedura rysuj¹ca zawartoœæ sceny
void Game::drawScene(GLFWwindow* window,mat4 &V, mat4 &P, Object &cube,Object &track, Car &player, Object tree[amount_of_trees], Car &enemy) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    sp->use();

//Swiatla
//----------------------------------------------------------------------------------------------------------------------

   float light_position[] = {
    0, 10000, -10, 1,
    0, 10000, 0, 1,
    0, 10000, 10, 1,
    0, 10000,20, 1
    };

    glUniform4fv(sp->u("lp"), amount_of_lights, light_position);
//----------------------------------------------------------------------------------------------------------------------


//Renderowanie obiektów
//----------------------------------------------------------------------------------------------------------------------
    player.render(V, P, sp);
    player.getMarkup()->getArrow()->render(V, P, sp);

    enemy.render(V, P, sp);
    cube.render(V, P, sp);
    track.render(V, P, sp);

    for(int i = 0 ; i < amount_of_trees; i++)
    {
        tree[i].render(V, P, sp);
    }

//----------------------------------------------------------------------------------------------------------------------


    glfwSwapBuffers(window); //Przerzuæ tylny bufor na przedni
}


void Game::moving(mat4 &V,  Car &player)
{
    bool camera_back = true;
    //obsluga kamery przy poruszaniu sie
    if (player.isMoving() == 1)
    {
        if (turnLeft)   //i jednoczesnie A
        {
            player.turnLeft();    //skrec gracza
            if ( angle_around_player >= -max_angle)  //skrec kamere
            {
                if ( speed_angle == 0)
                angle_around_player -= changing_angle;
            }
        camera_back = false;
        } else
	    if (turnRight) //i jednoczesnie D
        {
            player.turnRight();   //skrec gracza
            if (angle_around_player <= max_angle)    //skrec kamere
            {
                if ( speed_angle == 0)
                angle_around_player += changing_angle;
            }
        camera_back = false;
        }
    } else
    if (player.isMoving() == -1)
    {
        if (turnLeft)   //i jednoczesnie A
        {
            player.turnRight();   //skrec gracza
            if (angle_around_player < max_angle)    //skrec kamere
            {
                if ( speed_angle == 0)
                angle_around_player += changing_angle;
            }
            camera_back = false;
        } else
        if (turnRight)  //i jednoczesnie D
        {
            player.turnLeft();
            if ( angle_around_player > -max_angle)  //skrec kamere
            {
                if ( speed_angle == 0)
                angle_around_player -=  changing_angle;
            }
        camera_back = false;
        }
    }


         if (camera_back)
        {
            if (angle_around_player > 0)
            {
                angle_around_player -= 2*changing_angle;
            }
            if (angle_around_player < 0)
            {
                angle_around_player += 2*changing_angle;
            }
        }
        if (angle_around_player > 0)
            angle_around_player -= speed_angle;
        if (angle_around_player < 0)
            angle_around_player +=speed_angle;


        if (turnLeft)
        {
            player.turnWheelLeft();
        }
        if (turnRight)
        {
            player.turnWheelRight();
        }
        if (!turnLeft && !turnRight)  //prostuj ko³a
        {
            if (player.getWheelRotation() > 0)
            {
                player.turnWheelRight();
            }
            if (player.getWheelRotation() < 0)
            {
                player.turnWheelLeft();
            }
        }

     if (goPlayer)       //jesli trzyma W
    {
        player.move(1);  //rusz gracza
    } else
    if (backPlayer)     //jesli trzyma S
    {
        player.move(2);
    } else
    {
        player.move(0);
    }



    setCamera(V, player);

}


void Game::game( Object &cube, Object &track,Car &player,Object tree[amount_of_trees], Car &enemy)
{
    enemy.AI();
    player.checkpointReached();
    for(int i = 0 ; i < amount_of_trees; i++)   //kolizja z drzewami
    {
        if ( collision(player, tree[i])   )
        {
            player.setV(-0.2);
        }
    }
    float help;
    if ( collision(player, *enemy.getBody() )  ) //kolizja gracza z enemy
    {
        help = player.getV();
        player.setV( enemy.getV() );
        enemy.setV(help);
    }

    if (abs(player.getPosition().x) >= 70 || abs(player.getPosition().z) >= 125)
    {
        player.setV(-0.75);
    }
}
