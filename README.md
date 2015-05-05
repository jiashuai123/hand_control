# Contrôle par geste d'un drone #

**Extrait de la présentation du projet**

« On s'intéresse dans ce projet à contrôler un drone à l'aide la main. On utilisera pour ce faire une kinect, placée à l'horizontal, au dessus de laquelle on placera la main du contrôleur. La kinect fournit des informations sur la profondeur des objets placés en face d'elle. On peut alors régresser un plan sur les échantillons et utiliser deux inclinaisons et la distance pour contrôler le roulis, le tangage et l'altitude d'un drone. »

## Installation des dépendances ##
```
#!sh
sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu trusty main" > /etc/apt/sources.list.d/ros-latest.list'
wget https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -O - | sudo apt-key add -
sudo apt-get update
sudo apt-get install ros-indigo-desktop-full ros-indigo-freenect-stack ros-indigo-ardrone-autonomy libncursesw5-dev
```
## Utilisation du dépôt ##

Après avoir créé un espace de travail catkin :

```
#!sh
source /opt/ros/indigo/setup.bash
mkdir -p ~/hand_control_ws/src
cd ~/hand_control_ws/src
catkin_init_workspace
```
vous devez cloner le dépôt de telle sorte que le dossier «hand_control» se situe dans le dossier «~/hand_control_ws/src/», par exemple :

```
#!sh
cd ~/hand_control_ws/src
git clone git@bitbucket.org:_Luc_/hand_control.git
# ou bien : git clone https://username@bitbucket.org/_Luc_/hand_control.git # (changer username)
```
Le contenu du dépôt se trouve alors dans «~/hand_control_ws/src». Il est ensuite possible de compiler :

```
#!sh
cd ~/hand_control_ws
catkin_make
```

Puis pour faciliter le développement : 
```
#!sh
source /opt/ros/indigo/setup.bash
source ~/hand_control_ws/devel/setup.bash
echo "source /opt/ros/indigo/setup.bash" >> ~/.bashrc
echo "source ~/hand_control_ws/devel/setup.bash" >> ~/.bashrc

```

### Important ###

Avant de coder, regarder :

- [les conventions d’écriture du code du projet ROS](http://wiki.ros.org/CppStyleGuide)
- [le guide du développeur](http://wiki.ros.org/DevelopersGuide)

Cf. le [Wiki](https://bitbucket.org/_Luc_/handcontrol/wiki/Home) pour le reste de la documentation et le résultat des recherches.
