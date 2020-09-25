# ZingA3
ZingA3 EVM Firmware

# 목적
1.0부터 1.24까지는 인프리즘 이정민 선임으로부터 전달받은 소스들이다.
git으로 관리되고 있지 않아 이전의 버전들도 모두 git에 등록하고 tag를 붙여 이전 버전을 추적가능하게 한다.
아울러 1.24버전을 시작점으로 해서 Fx3SelfStudy에서 작업해 오던 내용들을 이곳으로 이전하여 향후 ZingA3 FW개발 내용들을 이곳에서 통합 관리한다

# 버전별 feature
## version : 1.0
- EVM 용 F/W 1차 version

## version : 1.21
- 안정성 보완
- Zing Reg R/W 방식 변경
- USB connection은 Zing 초기화 이후 시행되도록
- EP0 통한 management frame 송수신 기능 추가

## version : 1.22
- application.h 에서 RF / Serdes path 설정 가능하도록
- RF/Serdes path 설정 8bit bus width 에서 동작 확인

## version : 1.23
- gpio57 test code 제거 (usb c type flippable mux)
- ZING_EVMA3_V1.1 보드에서 동작 확인

## version : 1.24
- AFC 수정

## version : 1.25
- dma를 별도의 모듈로 구현
- git commit 해시 표시
- 프로젝트 구조 변경
- 코드 refactoring
  
## version : 1.26
- f/w 프로젝트를 A3와 tests로 구분
- 모듈 tests 추가 (i2c,pib,zing)
- 코드 refactoring
