# Summer Night Campsite 3D Scene

OpenGL을 이용해 여러 OBJ 모델을 불러오고, 하나의 여름밤 캠핑장 장면으로 구성한 컴퓨터그래픽스 기말 프로젝트입니다.

캠프파이어를 중심으로 텐트, 캠핑카, 피크닉 테이블, 랜턴, 수박, 나무, 돌, 통나무, 강아지, 나비 등을 배치했습니다. 모델마다 크기와 방향이 달라서 각 오브젝트의 위치, 회전, 스케일을 직접 조정하며 하나의 캠핑장처럼 보이도록 구성했습니다.

## Project Overview

| 항목               | 내용                                                                         |
| ---------------- | -------------------------------------------------------------------------- |
| Project          | Computer Graphics Final Project                                            |
| Theme            | Summer Night Campsite 3D Scene                                             |
| Language         | C++                                                                        |
| Graphics Library | OpenGL / freeglut                                                          |
| Model Format     | OBJ                                                                        |
| Main Features    | OBJ 모델 로딩, 텍스처 적용, 모델 배치, 애니메이션, Shading / Subdivision / Simplification 데모 |

## Preview

| Full Scene with Shading                                                                                                                  | Subdivision Demo                                                                                                                      | Simplification Demo                                                                                                                             |
| ---------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------- |
| <img height="180" alt="Full Scene with Shading" src="https://github.com/user-attachments/assets/63dd6165-07cd-491a-bfea-12b5180e2112" /> | <img height="180" alt="Dog Subdivision Demo" src="https://github.com/user-attachments/assets/402817bb-733e-42b9-b2cf-86773a770ae2" /> | <img height="180" alt="Watermelon Simplification Demo" src="https://github.com/user-attachments/assets/62144aeb-5f23-4971-bf83-229480aebfee" /> |


## Features

* 여러 OBJ 모델 로딩 및 렌더링
* 텍스처 적용
* 모델별 위치, 크기, 회전 조정
* 캠핑장 배경 구성
* 그림자와 glow 효과를 이용한 시각적 보완
* 나비 이동 및 강아지 방향 변화 애니메이션
* Shading, Subdivision, Simplification 데모 기능 구현

## Implemented Topics

### Shading

OpenGL lighting과 smooth normal을 사용하여 장면에 명암을 적용했습니다.
캠프파이어, 랜턴, 캠핑카 창문에는 glow 효과를 추가하여 밤 캠핑장 분위기를 표현했습니다.

### Subdivision

강아지 모델에 midpoint subdivision을 적용했습니다.
원본 low-poly 모델과 subdivided 모델을 전환해서 비교할 수 있도록 했습니다.

### Simplification

수박 모델에 vertex clustering 방식의 simplification을 적용했습니다.
단순화된 모델에는 wireframe을 함께 표시하여 변화가 보이도록 했습니다.

## Demo Controls

| Key / Mouse      | Description                      |
| ---------------- | -------------------------------- |
| Left mouse drag  | Rotate scene                     |
| Right mouse drag | Move scene                       |
| Mouse wheel      | Zoom in / out                    |
| `1`              | Shading on/off                   |
| `2`              | Dog subdivision on/off           |
| `3`              | Watermelon simplification on/off |
| `R`              | Reset view                       |
| `ESC`            | Exit                             |

## Project Structure

```text
.
├── Kinect_Sample_Re/
│   ├── Renderer.cpp
│   ├── Renderer.h
│   └── stb_image.h
├── assets/
│   ├── background/
│   ├── butterfly/
│   ├── campfire/
│   ├── camping_car/
│   ├── dog/
│   ├── ground/
│   ├── lantern/
│   ├── picnic_table/
│   ├── rock/
│   ├── tent/
│   ├── tree/
│   ├── Watermelon/
│   └── wood/
├── gltest.sln
├── .gitignore
└── README.md
```

## How to Run

1. Visual Studio에서 `gltest.sln`을 엽니다.
2. `Kinect_Sample_Re` 프로젝트를 빌드합니다.
3. `assets` 폴더가 현재 저장소 구조처럼 루트 경로에 있는지 확인합니다.
4. Debug x64 또는 Release x64 환경에서 실행합니다.
5. 실행 후 `1`, `2`, `3` 키를 눌러 각 데모 기능을 확인할 수 있습니다.

## Notes

기존 수업 템플릿 구조를 유지하면서 `Renderer.cpp`와 `Renderer.h`를 중심으로 기능을 확장했습니다.
OBJ 모델마다 크기와 축 방향이 달라서 각 모델별 transform 값을 직접 조정했습니다.
MTL 전체 파서를 구현하기보다는 프로젝트에 필요한 주요 텍스처를 직접 연결하는 방식으로 처리했습니다.
