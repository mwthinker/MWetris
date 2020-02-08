#ifndef INPUT_H
#define INPUT_H

namespace tetris {

	struct Input {
		bool rotate : 1;
		bool down : 1;
		bool downGround : 1;
		bool left : 1;
		bool right : 1;
	};

}

#endif // INPUT_H
