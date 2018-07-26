/*! \file stopwatch.c
  
  \brief Stopwatch application.
   
  This is a simple stop watch, which begins counting as the + key is
  hit and clears the count when the 0 key is pressed.  Because the
  RTC is busy with the real time, we could in quarter seconds as the
  display is updated.

  We do not count time or store the count when in other applications.
*/

#include "api.h"


static int counting=0;

/* It's a crying shame, but you'll break the code if you increase this
   count to a long, because repeated divisions in rendering will take
   longer than the ~8k cycles we have per interrupt when running at
   32kHz and the stopwatch will run too slow.

   To work around this, we maintain the count of hours separately from
   the general second count.
 */
static unsigned int count, hour;


//! Entry to the stopwatch app.
void stopwatch_init(){
  //Zero the count.
  count=0;
  hour=0;
  //Start off without counting.
  counting=0;
}

//! Exit form the stopwatch app.
int stopwatch_exit(){
  //Give up without a fight when the mode button is pressed.
  return 0;
}

//! A button has been pressed for the stopwatch.
int stopwatch_keypress(char ch){
  /* For now, we support only two buttons.  + stops and starts the
     count, while 0 resets the counter.
   */
  
  switch(ch){
    case '+':
      counting=!counting;
      break;
    case '0':
      //Clear both the second counter and the hour counter.
      count=0;
      hour=0;
      break;
  }
  
  //Stopwatch uses rendering frequency to count time, so we don't
  //redraw after a keypress.
  return 0;
}

//! Renders the low bits of the count in quarter seconds.
static const char subs[]={0, 0x25, 0x50, 0x75};

//! Draws the time.
void stopwatch_draw_time(){
  unsigned int min, sec,
    minhex, sechex, hourhex, subhex;

  /* The stopwatch is special in that it never times out.  Be very
     careful when doing this, because a minor bug might kill the
     battery.
   */
  app_cleartimer();
  
  //Increment the count if we're counting.
  if(counting)
    count++;
  
  //Divide the count.
  min=((count>>2)/60)%60;
  sec=(count>>2)%60;
  subhex=subs[count&3];

  //Handle hour rollover.
  if(((count>>2)/60)>60){
    count=0;
    hour++;
    hourhex=int2bcd(hour);
  }
  
  if(hour){
    //Only draw if the subseconds have changed.
    if(!subhex){
      hourhex=int2bcd(hour);
      minhex=int2bcd(min);
      sechex=int2bcd(sec);
      
      lcd_digit(7,hourhex>>4);
      lcd_digit(6,hourhex&0xF);
      lcd_cleardigit(5); //Space
      setcolon((count>>1)&1);
      lcd_digit(4,minhex>>4);
      lcd_digit(3,minhex&0xF);
      lcd_cleardigit(2); //Space
      lcd_digit(1,sechex>>4);
      lcd_digit(0,sechex&0xF);
    }
  }else{
    //Draw the subsecond first.
    lcd_digit(1,subhex>>4);
    lcd_digit(0,subhex&0xF);
    
    //Only draw the rest if the subseconds have changed.
    if(!subhex || count==1){
      minhex=int2bcd(min);
      sechex=int2bcd(sec);
      
      lcd_digit(7,minhex>>4);
      lcd_digit(6,minhex&0xF);
      lcd_cleardigit(5); //Space
      setcolon((count>>1)&1);
      lcd_digit(4,sechex>>4);
      lcd_digit(3,sechex&0xF);
      lcd_cleardigit(2); //Space
    }
  }
}


//! Draw the stopwatch app and handle its input.
void stopwatch_draw(){
  stopwatch_draw_time();
}
