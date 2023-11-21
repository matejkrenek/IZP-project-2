<!-- Improved compatibility of back to top link: See: https://github.com/othneildrew/Best-README-Template/pull/73 -->

<a name="readme-top"></a>

<!--
*** Thanks for checking out the Best-README-Template. If you have a suggestion
*** that would make this better, please fork the repo and create a pull request
*** or simply open an issue with the tag "enhancement".
*** Don't forget to give the project a star!
*** Thanks again! Now go create something AMAZING! :D
-->

<div align="center">
  <h2 align="center">C Scaffolding</h2>

  <p align="center">
    Easy to use scaffolding structure for c projects
    <br />
    <a href="https://github.com/matejkrenek/c-scaffolding/issues">Report Bug</a>
    ·
    <a href="https://github.com/matejkrenek/c-scaffolding/issues">Request Feature</a>
  </p>
</div>

<br/>

<!-- TABLE OF CONTENTS -->

### Table of contents

  <li>
    <a href="#installation">Installation</a>
  </li>
  <li><a href="#usage">Usage</a></li>
  <li><a href="#contact">Contact</a></li>
  <li><a href="#acknowledgments">Acknowledgments</a></li>

## Installation

Now when all necessary tools are downloaded you can run the toolchain on your machine.

1. Clone the repo

   ```sh
   git clone https://github.com/matejkrenek/c-scaffolding.git {project_name}
   ```

2. CD to the cloned directory

   ```sh
   cd {project_name}
   ```
   
3. Compile and run your code using make commands documented in <a href="#usage">Usage</a> section

<!-- USAGE EXAMPLES -->

## Usage

- make commands that you can use to trigger functions

  ```sh
  make <command> [ARGUMENT1=value1] [ARGUMENT2=value2] [...]
  ```

  | Command   | Functionality                                                          |
  | --------- | ---------------------------------------------------------------------- |
  | "help"    | Show all the informations about project and what commands you can use  |
  | "init"    | Create project configuratin file                                       |
  | "compile" | Compile your C program                                                 |
  | "run"     | Run your compiled C program                                            |
  | "", "all" | Compile and run your C program                                         |

<!-- CONTACT -->

## Contact

Matěj Křenek - [mate23.krenek@gmail.com](mailto:mate23.krenek@gmail.com)

Project Link: [https://github.com/matejkrenek/c-scaffolding](https://github.com/matejkrenek/c-scaffolding)

<!-- ACKNOWLEDGMENTS -->

## Acknowledgments

S/O to chatgpt

<p align="right">(<a href="#readme-top">Back to top</a>)</p>

## Notes and Examples
- Each field in map is represented by 3 bits
  - x2x1x0 (x0 - left wall, x1 - right wall, x2 - top/bottom wall)

## Map Analysis
6 7
1 4 4 2 5 0 6
1 4 4 0 4 0 2
1 0 4 0 4 6 1
1 2 7 1 0 4 2
3 1 4 2 3 1 2
4 2 5 0 4 2 5

- way out from row *6*, column *1*
6,1 - walls 100 on left bottom wall
6,2 - wall 010 on right
5,2 - wall 001 on top
5,1 - wall 011 on right and bottom - cant go here
5,3 - wall 100 on top
5,4 - wall 010 on right
5,5 - wall 011 on right and left - cant go here
6,4 - wall 001 on left
.
.
. 

