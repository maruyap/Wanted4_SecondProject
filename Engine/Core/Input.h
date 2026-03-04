#pragma once

#include "Common/Common.h"

namespace Wanted
{
	class WANTED_API Input
	{
		friend class Engine;

		// 데이터.
		// Down/Up/Key
		struct KeyState
		{
			// 현재 키가 눌렸는지.
			bool isKeyDown = false;

			// 이전에 키가 눌렸는지.
			bool wasKeyDown = false;
		};

	private:
		Input();
		~Input();

	public:
		// 입력 확인 함수.
		// 이전에 입력이 안됐는데, 현재 입력이 됐으면 1번 호출.
		bool GetKeyDown(int keyCode);

		// 이전에 입력이 됐는데, 현재 입력이 취소됐으면 1번 호출.
		bool GetKeyUp(int keyCode);

		// 현재 눌려있으면 반복 호출.
		bool GetKey(int keyCode);

		// 전역적으로 접근하는 함수.
		static Input& Get();

	private:
		// 입력 처리 함수.
		void ProcessInput();

		// 기존 입력 저장 함수.
		void SavePreviousInputStates();

	private:
		// 키 상태 저장용 배열.
		KeyState keyStates[255] = { };

		// 전역적으로 접근하도록 만들기 위한 전역 변수.
		static Input* instance;
	};
}
