#include "evolution.h"
#include <Evolution/Parameters/evolutionParameters.h>
#include <fstream>
#include <iostream>

extern EvolutionParameters evolutionParameters;

Evolution::Evolution()
{
    infoBar.initInfoBar(
        evolutionParameters.infoBarStartX,
        evolutionParameters.infoBarStartY,
        evolutionParameters.infoBarEndX,
        evolutionParameters.infoBarEndY
    );
    toolsBar.initToolsBar(
        evolutionParameters.toolsBarStartX,
        evolutionParameters.toolsBarStartY, 
        evolutionParameters.toolsBarEndX, 
        evolutionParameters.toolsBarEndY
    );
    squareBar.initSquareBar(
        evolutionParameters.squareBarStartX,
        evolutionParameters.squareBarStartY, 
        evolutionParameters.squareBarEndX, 
        evolutionParameters.squareBarEndY
    );
}


void Evolution::run()
{
    timeout(evolutionParameters.delayDuration);

    currentScreen.clearScreen();

    evolutionParameters.turn = 0;
    evolutionParameters.score = 0;
    evolutionParameters.generation = 1;
    evolutionParameters.snakeIdCounter = 1;
    evolutionParameters.aliveSnakes = 0;

    initSnakes();

	evolutionParameters.gameOn = true;

    while(evolutionParameters.gameOn){
        key = currentScreen.controllHandler();

        if (!toolsBar.pauseOn){
            turn();
            evolutionParameters.hightTurnsLeft--;
            evolutionParameters.turn++;
            evolutionParameters.time++;
            snakeTmp = snakes;
            while(snakeTmp->nextSnake){
                if (snakeTmp->currentSnake->isAlive)
                {
                    snakeTmp->currentSnake->setVector(key);
                }
                snakeTmp = snakeTmp->nextSnake;
            }
        }

        if (evolutionParameters.aliveSnakes < 1){
            evolutionParameters.hightTurnsLeft = evolutionParameters.turnsToDeath;
            evolutionParameters.snakeIdCounter = 1;
            evolutionParameters.bestFood = 0;
            evolutionParameters.generation++;
            evolutionParameters.turn = 0;
            if (evolutionParameters.score > evolutionParameters.theBestScore){
                evolutionParameters.theBestScore = evolutionParameters.score;
            }
            evolutionParameters.score = 0;
            evolveSnakes();
        }

        drawScreen();
        drawStuff();
        toolsBar.menuControllHandler(key);
        if (!evolutionParameters.gameOn){
            deleteSnakes(snakes);
        }
    }
    currentScreen.clearScreen();
}


void Evolution::turn()
{
    snakeTmp = snakes;
    while(snakeTmp->nextSnake){
        if (snakeTmp->currentSnake->isAlive)
        {
            snakeTmp->currentSnake->score++;
            if (snakeTmp->currentSnake->score > evolutionParameters.score){
                evolutionParameters.score = snakeTmp->currentSnake->score;
                evolutionParameters.bestSnakeId = snakeTmp->currentSnake->snakeId;
            }
            snakeTmp->currentSnake->move();
        }
        snakeTmp = snakeTmp->nextSnake;
    }

    currentScreen.endFrame();
    currentScreen.clearScreen();
}


void Evolution::drawScreen()
{
    infoBar.drawInfoBar();
    toolsBar.drawToolsBar();

    snakeTmp = snakes;
    do {
        if (snakeTmp->currentSnake->isAlive){
            snakeTmp->currentSnake->drawField();
            squareBar.drawMap(snakeTmp->currentSnake->field);
            break;
        }
        snakeTmp = snakeTmp->nextSnake;
    } while (snakeTmp->nextSnake);
}


void Evolution::drawStuff()
{
    snakeTmp = snakes;
    while(snakeTmp->nextSnake){
        if (snakeTmp->currentSnake->isAlive)
        { 
            snakeTmp->currentSnake->drawStuff();
        }
	    snakeTmp = snakeTmp->nextSnake;
   } 
}


void Evolution::initSnakes()
{
    snakes = new snakesList;
    snakeTmp = snakes;

    for (int count = 0; count < evolutionParameters.countOfSnakes; count++)
	{
        SnakeEvoModel *newSnake = new SnakeEvoModel;
        newSnake->init(
            evolutionParameters.fullFieldSizeX / 2,
            evolutionParameters.fullFieldSizeY / 2,
            evolutionParameters.snakeLength,
            evolutionParameters.snakeOneBodyColor
        );
        snakeTmp->currentSnake = newSnake;
        snakeTmp->nextSnake = new snakesList;
        snakeTmp = snakeTmp->nextSnake;
        snakeTmp->nextSnake = nullptr;
    }
    squareBar.initField(snakes->currentSnake->field);
}


void Evolution::evolveSnakes()
{
    getBest();
    snakeSecondTmp = new snakesList;
    snakeTmp = snakeSecondTmp;

    int counter = 1;
    for (int count = 0; count < evolutionParameters.countOfSnakes; count++)
	{
        SnakeEvoModel *newSnake = new SnakeEvoModel;
        if ((evolutionParameters.fullFieldSizeY / 2) + (count * 2) + counter > (evolutionParameters.fullFieldSizeY - 3)){
            counter = 1;
        }
        newSnake->init(
            evolutionParameters.fullFieldSizeX / 2,
            evolutionParameters.fullFieldSizeY / 2,
            evolutionParameters.snakeLength,
            evolutionParameters.snakeOneBodyColor
        );

        int parentOneNum = 0;
        int parentTwoNum = 0;

        int parentsCount = ((evolutionParameters.countOfSnakes / 10) * evolutionParameters.countOfBest / 10) + ((evolutionParameters.countOfSnakes / 10) * evolutionParameters.countOfWorst / 10);
        
        std::random_device random_device;
        std::mt19937 generator(random_device());
        std::uniform_int_distribution<> randGen(0, parentsCount - 1);

        while (parentOneNum == parentTwoNum) {
            parentOneNum = randGen(generator);
            parentTwoNum = randGen(generator);
        }

        #ifdef LOGS
            std::ofstream evolveSnake;
            evolveSnake.open("logs/evolveSnake.txt");
        #endif 

        snakeListTmp = parentSnakes;
        for (int countX = 0; countX < parentOneNum; countX++){
            snakeListTmp = snakeListTmp->nextSnake;
        }
        #ifdef LOGS
            evolveSnake << "PARENT 1 NEURON ID : " << snakeListTmp->currentSnake->network.firstLayer->currentNeuron.neuronId << "\n" << std::endl;
        #endif

        snakeListSecondTmp = parentSnakes;
        for (int countX = 0; countX < parentTwoNum; countX++){
            snakeListSecondTmp = snakeListSecondTmp->nextSnake;
        }
        #ifdef LOGS
            evolveSnake << "PARENT 2 NEURON ID : " << snakeListSecondTmp->currentSnake->network.firstLayer->currentNeuron.neuronId << "\n" << std::endl;
        #endif
        
        newSnake->network.mergeNetworks(parentSnakes->currentSnake->network.layersList, parentSnakes->currentSnake->network.layersList);
        
        snakeSecondTmp->currentSnake = newSnake;
        snakeSecondTmp->nextSnake = new snakesList;
        snakeSecondTmp = snakeSecondTmp->nextSnake;
        snakeSecondTmp->nextSnake = nullptr;
        counter++;
    }
    squareBar.initField(snakes->currentSnake->field);
    deleteSnakes(snakes);
    snakes = snakeTmp;
}


void Evolution::deleteSnakes(snakesList *firstSnake)
{
    snakeSecondTmp = firstSnake;
    while(snakeSecondTmp->nextSnake){
        snakeSecondTmp->currentSnake->deleteSnake();
        delete snakeSecondTmp->currentSnake;
        snakeSecondTmp->currentSnake = nullptr;
        snakeFoursTmp = snakeSecondTmp;
        snakeSecondTmp = snakeSecondTmp->nextSnake;
        delete snakeFoursTmp;
        snakeFoursTmp = nullptr;
    }
    delete snakeSecondTmp;
    snakeSecondTmp = nullptr;

    if (bestSnake){
        snakeListTmp = bestSnake;
        while(snakeListTmp->nextSnake){
            snakeListSecondTmp = snakeListTmp;
            snakeListTmp = snakeListTmp->nextSnake;
            delete snakeListSecondTmp;
            snakeListSecondTmp = nullptr;
        }
        delete snakeListTmp;
        bestSnake = nullptr;
    }

    if (parentSnakes){
        snakeListTmp = parentSnakes;
        while(snakeListTmp->nextSnake){
            snakeListSecondTmp = snakeListTmp;
            snakeListTmp = snakeListTmp->nextSnake;
            delete snakeListSecondTmp;
            snakeListSecondTmp = nullptr;
        }
        delete snakeListTmp;
        snakeListTmp = nullptr;
        parentSnakes = nullptr;
    }
}


void Evolution::getBest()
{
    snakeTmp = snakes;
    bestSnake = new bestSnakesList;
    bestSnake->currentSnake = snakes->currentSnake;
    bestSnake->prevSnake = nullptr;
    bestSnake->nextSnake = nullptr;

    while (snakeTmp->nextSnake){
        if (snakeTmp->currentSnake->score >= bestSnake->currentSnake->score){
            snakeListTmp = new bestSnakesList;
            snakeListTmp->currentSnake = snakeTmp->currentSnake;
            snakeListTmp->prevSnake = nullptr;
            snakeListTmp->nextSnake = bestSnake;
            bestSnake->prevSnake = snakeListTmp;
            bestSnake = snakeListTmp;
        } else {
            snakeListTmp = bestSnake;
            while(snakeListTmp->nextSnake){
                if (snakeTmp->currentSnake->score >= snakeListTmp->currentSnake->score){
                    if (snakeListTmp->nextSnake){
                        snakeListSecondTmp = new bestSnakesList;
                        snakeListSecondTmp->currentSnake = snakeTmp->currentSnake;
                        snakeListSecondTmp->prevSnake = snakeListTmp->prevSnake;
                        snakeListSecondTmp->nextSnake = snakeListTmp;
                        snakeListTmp->prevSnake->nextSnake = snakeListSecondTmp;
                        snakeListTmp->prevSnake = snakeListSecondTmp;
                        break;
                    } else {
                        snakeListSecondTmp = new bestSnakesList;
                        snakeListSecondTmp->currentSnake = snakeTmp->currentSnake;
                        snakeListSecondTmp->prevSnake = snakeListTmp;
                        snakeListSecondTmp->nextSnake = nullptr;
                        snakeListTmp->nextSnake = snakeListSecondTmp;
                        break;
                    }
                }
                snakeListTmp = snakeListTmp->nextSnake;
                if (!snakeListTmp->nextSnake){
                    snakeListSecondTmp = new bestSnakesList;
                    snakeListSecondTmp->currentSnake = snakeTmp->currentSnake;
                    snakeListSecondTmp->prevSnake = snakeListTmp;
                    snakeListSecondTmp->nextSnake = nullptr;
                    snakeListTmp->nextSnake = snakeListSecondTmp;
                    break;
                }
            }
            
        }
        snakeTmp = snakeTmp->nextSnake;
    }

    snakeListTmp = bestSnake;
    while(snakeListTmp->nextSnake){
        snakeListTmp = snakeListTmp->nextSnake;
    }
    worstSnake = snakeListTmp->prevSnake;

    snakeListTmp = bestSnake;
    parentSnakes = new bestSnakesList;
    snakeListSecondTmp = parentSnakes;
    snakeListSecondTmp->prevSnake = nullptr;
    for (int count = 0; count < ((evolutionParameters.countOfSnakes / 10) * evolutionParameters.countOfBest / 10); count++){
        snakeListSecondTmp->currentSnake = snakeListTmp->currentSnake;
        snakeListSecondTmp->nextSnake = new bestSnakesList;
        snakeListSecondTmp->nextSnake->prevSnake = snakeListSecondTmp;
        snakeListSecondTmp = snakeListSecondTmp->nextSnake;
        snakeListSecondTmp->nextSnake = nullptr;

        snakeListTmp = snakeListTmp->nextSnake;
    }

    snakeListTmp = worstSnake;
    for (int count = 0; count < ((evolutionParameters.countOfSnakes / 10) * evolutionParameters.countOfWorst / 10); count++){
        snakeListSecondTmp->currentSnake = snakeListTmp->currentSnake;
        snakeListSecondTmp->nextSnake = new bestSnakesList;
        snakeListSecondTmp->nextSnake->prevSnake = snakeListSecondTmp;
        snakeListSecondTmp = snakeListSecondTmp->nextSnake;
        snakeListSecondTmp->nextSnake = nullptr;

        snakeListTmp = snakeListTmp->nextSnake;
    }

    snakeListTmp = parentSnakes;
    while(snakeListTmp->nextSnake){
        snakeListTmp = snakeListTmp->nextSnake;
    }
}