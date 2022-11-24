#pragma once

/*/
인터페이스는 glog와 같은 사용인터페이스
	logger myLogger(WARNNING,	"c:\\어디어디",		CSV | TXT);
				//	로그 레벨,	 저장 위치,			출력형태 = default, TXT; (비트연산 이용)

	LOG(INFO) << "어쩌구저쩌구";				// <- logger.cpp에 있는 기본 로거에 자동저장, extern 이용
												// 위치는 헤더에 전역으로 정의되어있음, 바꾸는 메소드 지원;
	LOG(WARNNING, myLogger) << "저쩌구 어쩌구"	// <- 위에서 선언한 myLogger에 저장

	LOG(WARNNING, "c:\\어디어디", CSV | TXT)) << "저쩌구 어쩌구"	// <- myLogger 없이, 임시객체를 이용한 로그 메세지 생성 후 로깅 큐에 넣기



	cpp 파일별 별도 로깅 방법
		해당 cpp에서 myLogger 를 선언하여 이용함 (ifdef 이용 구조 포기)
			-> 왜냐면 로그가 어디에 어떻게 남았으면 하는 시점은 정확히 "로그를 남기는 코드를 적는 순간" 이기 때문에
				한 파일 내에서도 전역적인 설정을 가지고 가는게 아니라 자기가 원하는 로그를 남기는 순간에 myLogger 를 이용하자
				또는 로깅만 단일로 처리할 매크로를 만들자.





	스레드 세이프티 로깅 대책
		1. 로거 인스턴스에서 입력을 기준으로 로깅 메세지 구조체 만들기
		2. 락프리 글로벌 큐 이용하자, 위에서 만들어진 메세지 구조체 넣기
		3. 로깅용 스레드 풀 이용 (1개면 될듯?, 테스트 필요함, 굳이 파일 출력하는 놈들끼리 경쟁할 필요가 있을까?) 해 큐에서 메세지 꺼내서 쓰기






	실제 출력되는 로그파일은
	TXT:
		[__TIME__][__LEVEL__] __MESSAGE__ [__COMMENT__] at [__CODE__]

		ex1) COMMENT 까지 활성화시
		[오후 1:21 2022-08-08][ERROR] 소켓 에러 10000 [//10000 : 어떤에러, 11000 :저떤 에러, 그외 : 발생시 연락바람 YWlee@~~~.com] at [netWorkLib.cpp 127 line]
		[오후 1:21 2022-08-08][ERROR] 소켓 에러 10002 [//10000 : 어떤에러, 11000 :저떤 에러, 그외 : 발생시 연락바람 YWlee@~~~.com] at [netWorkLib.cpp 129 line]

		ex2) COMMENT 제거시
		[오후 1:21 2022-08-08][ERROR] 소켓 에러 10000 at [netWorkLib.cpp 127 line]
		[오후 1:21 2022-08-08][ERROR] 소켓 에러 10002 at [netWorkLib.cpp 129 line]



	CSV
		__TIME__, __LEVEL__, __MESSAGE__, __COMMENT__, __CODE__
		"오후 1:21 2022-08-08", "1", "소켓 에러 10000", "//10000 : 어떤에러, 11000 :저떤 에러, 그외 : 발생시 연락바람 YWlee@~~~.com", "netWorkLib.cpp 127 line"


/*/





