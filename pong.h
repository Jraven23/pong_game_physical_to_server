
#define TOP_POS 300
#define BOTTOM_POS 500
#define LEFT_POS 200
#define RIGHT_POS 600
#define RAQUET_HOR_MARGIN 2

struct Position {
  Position(float x, float y) : x(x), y(y) {}
  float x;
  float y;
};

struct Velocity {
  Velocity(float x, float y) : x(x), y(y) {}
  float x;
  float y;
};

struct Ball {
  Ball(Position pos,Velocity vel) : pos(pos), vel(vel){}
  Position pos;
  Velocity vel;
};

struct Pong {
  Pong(Ball ball, int player1Pos,int player2Pos, int raquetLength, int player1points, int player2points) : ball(ball), player1Pos(player1Pos), player2Pos(player2Pos),raquetLength(raquetLength),player1Points(0),player2Points(0){}
  Ball ball;
  int player1Pos;
  int player2Pos;
  int raquetLength;
  int player1Points;
  int player2Points;
};

void player_within_limits(int& pos, int len)
{
  if ((pos+len/2)>BOTTOM_POS) //TOP is < than BOTTOM in pixels
    pos = BOTTOM_POS-len/2;
  if ((pos-len/2)<TOP_POS) //TOP is < than BOTTOM in pixels
    pos = TOP_POS+len/2;    
}

void ball_update_vel(Pong* pong)
{
  if(pong->ball.pos.y >= BOTTOM_POS || pong->ball.pos.y <= TOP_POS)
  {
    pong->ball.vel.y = - pong->ball.vel.y;
  }
  if(pong->ball.pos.x >= RIGHT_POS-RAQUET_HOR_MARGIN)
    if(pong->ball.pos.y < pong->player2Pos+pong->raquetLength/2 && pong->ball.pos.y > pong->player2Pos-pong->raquetLength/2)
      pong->ball.vel.x = - pong->ball.vel.x;
  if(pong->ball.pos.x <= LEFT_POS+RAQUET_HOR_MARGIN)
    if(pong->ball.pos.y < pong->player1Pos+pong->raquetLength/2 && pong->ball.pos.y > pong->player1Pos-pong->raquetLength/2)
      pong->ball.vel.x = - pong->ball.vel.x;
}

void ball_update_pos(Ball* b, float dt)
{  
  b->pos.x = b->pos.x+b->vel.x*dt; 
  b->pos.y = b->pos.y+b->vel.y*dt; 
}

void ball_update(Pong* pong, float dt)
{
  ball_update_vel(pong);
  ball_update_pos(&(pong->ball),dt);
}

void resetBall(Ball* b)
{
  b->pos.x = LEFT_POS+(RIGHT_POS-LEFT_POS)/2;
  b->pos.y = TOP_POS+(BOTTOM_POS-TOP_POS)/2;
  //generate initial direction for velocity randomly
  float percentage = random(30,71);
  if(random(0,2)) //X direction
    b->vel.x = percentage/100;
  else
    b->vel.x = -percentage/100;
  if(random(0,2)) //Y direction
    b->vel.y = 1-percentage/100;
  else
    b->vel.y = -(1-percentage/100);
}

void goal_check(Pong* pong)
{
  if(pong->ball.pos.x < LEFT_POS)
  {
    pong->player2Points++;
    resetBall(&(pong->ball));
  }
  if(pong->ball.pos.x > RIGHT_POS)
  {
    pong->player1Points++;
    resetBall(&(pong->ball));
  }
}