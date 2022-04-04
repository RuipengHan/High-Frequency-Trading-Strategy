# group_01_project

This is the base git project for group 01 of IE498 HFT in Spring 2022.

## Teammates

Yihong Jian - yihongj2@illinois.edu (Team Leader)

Ruipeng Han - ruipeng2@illinois.edu

Zihan zhou - zihanz12@illinois.edu

Tomoyoshi (Tommy) Kimura - tkimura4@illinois.edu

## Environment

Standard development environment is provided using vagrant box.

### Software included
- Strategy Studio
- Python 3.7.11
  - Additional Packages should be included in ```requirements.txt```.

Please create a issue for additional softwares so that we can put it in vagrant provisioning.

### Usage:

1. Install Vagrant and virtualbox.
2. Download box using this [link](https://uofi.box.com/s/wlyq6b23k41dbw1bz7sfff631osp1049) that requires login from your university account.
3. Set directory of box file to environment variable ```SS_BOX```, for example:
    ```   bash
    export SS_BOX="/home/user/IE498hftGroup1.box
4. Set directory of strategy studio ```SS_LICENSE```, for example:
    ```   bash
    export SS_LICENSE="/home/user/license.txt
5. ``` bash
   vagrant up
6. Then use ```vagrant ssh``` to connect to VM

## Project structure
### Components
- **Market data parser**

    All the data parser (Nasdaq/IEX/alpaca) should be placed under ```parser``` folder. Implementation should follow the interface in ```parser_base.py```.

- **Strategy**

    All the strategies codes (.h, .cpp, makefile) should be included under ```strategy``` folder.

- **Analysis/visualization**

    Code should be included in ```visualization``` folder.
### Quality assurance
- **Code Sanity**
  - Python code will be checked by ```PyLint``` under PEP8 standard.
  - CPP checkstyle - TBD
- **Unit Testing**
  - TBD