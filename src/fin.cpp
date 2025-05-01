#include "fin.h"
uint8_t L[8] = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3C};
uint8_t field_matrix[8] = {0};
uint8_t gravity = 1; //2 = rotated left 90 degrees, 1 = 0 degrees, 0 = rotate right 90 degres
//piece properties
#define NUMBER_PIECES 7
bool piece_array[NUMBER_PIECES][4][3][3] =
{
    //I
    {
        {{0, 1, 0},
         {0, 1, 0},
         {0, 1, 0}},

        {{0, 0, 0},
         {1, 1, 1},
         {0, 0, 0}},

        {{0, 1, 0},
         {0, 1, 0},
         {0, 1, 0}},

        {{0, 0, 0},
         {1, 1, 1},
         {0, 0, 0}}
    },
    //Z
    {
        {{0, 0, 0},
         {1, 1, 0},
         {0, 1, 1}},

        {{0, 1, 0},
         {1, 1, 0},
         {1, 0, 0}},

        {{1, 1, 0},
         {0, 1, 1},
         {0, 0, 0}},

        {{0, 0, 1},
         {0, 1, 1},
         {0, 1, 0}}
    },
    //S
    {
        {{0, 0, 0},
         {0, 1, 1},
         {1, 1, 0}},

        {{1, 0, 0},
         {1, 1, 0},
         {0, 1, 0}},

        {{0, 1, 1},
         {1, 1, 0},
         {0, 0, 0}},

        {{0, 1, 0},
         {0, 1, 1},
         {0, 0, 1}}
    },
    //T
    {
        {{0, 0, 0},
         {1, 1, 1},
         {0, 1, 0}},

        {{0, 1, 0},
         {1, 1, 0},
         {0, 1, 0}},

        {{0, 1, 0},
         {1, 1, 1},
         {0, 0, 0}},

        {{0, 1, 0},
         {0, 1, 1},
         {0, 1, 0}}
    },
    //L
    {
        {{0, 1, 0},
         {0, 1, 0},
         {0, 1, 1}},

        {{0, 0, 0},
         {1, 1, 1},
         {1, 0, 0}},

        {{1, 1, 0},
         {0, 1, 0},
         {0, 1, 0}},

        {{0, 0, 1},
         {1, 1, 1},
         {0, 0, 0}}
    },
    //J
    {
        {{0, 1, 0},
         {0, 1, 0},
         {1, 1, 0}},

        {{1, 0, 0},
         {1, 1, 1},
         {0, 0, 0}},

        {{0, 1, 1},
         {0, 1, 0},
         {0, 1, 0}},

        {{0, 0, 0},
         {1, 1, 1},
         {0, 0, 1}}
    },
    //N
    {
        {{0, 0, 0},
         {0, 1, 1},
         {0, 1, 1}},

        {{0, 0, 0},
         {0, 1, 1},
         {0, 1, 1}},

        {{0, 0, 0},
         {0, 1, 1},
         {0, 1, 1}},

        {{0, 0, 0},
         {0, 1, 1},
         {0, 1, 1}},
    }
};

int8_t falling_piece_x_pos = 2;
int8_t falling_piece_y_pos = -2;
bool hit_floor = false;
uint8_t rotation = 0;
uint8_t current_peice = 0;

bool move_left = false;
bool move_right = false;
bool rotate_cw = false;
bool rotate_ccw = false;
bool fast_drop = false;

enum state_enum {PLAYING, LOST};
state_enum state = LOST;

uint8_t lost_count;
bool sand;

void WriteByte ( unsigned char data )
{
    for ( int i = 0; i < 8; i++ )
    {
        PORTH &= ~( 1 << CLK );
        if ( data & 0x80 )
        {
            PORTH |= ( 1 << DIN );
        }
        else
        {
            PORTH &= ~( 1 << DIN );
        }
        data <<= 1;
        PORTH |= ( 1 << CLK );
    }
}

void WriteData ( unsigned char addr, unsigned char data )
{
    PORTH &= ~( 1 << CS );
    WriteByte ( addr );
    WriteByte ( data );
    PORTH |= ( 1 << CS );
}

void InitLEDMatrix(void)
{
    WriteData ( 0x09, 0x00 ); //decoding
    //WriteData ( 0x0a, 0x08, 0x0a, 0x08 ); //reg brightness
    WriteData ( 0x0a, 0x00 ); //low brightness
    WriteData ( 0x0b, 0x07 ); //scan limit
    WriteData ( 0x0c, 0x01 ); //power down mode
    WriteData ( 0x0f, 0x01 ); //test display
    _delay_ms ( 200 );
    WriteData ( 0x0f, 0x00 ); //test display
    _delay_ms ( 200 );
    WriteData ( 0x0f, 0x01 ); //test display
    _delay_ms ( 200 );
    WriteData ( 0x0f, 0x00 ); //test display
    _delay_ms ( 200 );
}

uint16_t mi_random()
{
    static uint16_t seed = analogReadManual(4);
    static uint16_t a  = analogReadManual(3);
    static uint16_t c  = 5 * analogReadManual(4);
    static uint16_t m  = 11 * analogReadManual(5);

    seed = (a* seed + c) % m;
    return seed;
}


//game logic//
void reset()
{
    falling_piece_x_pos = 2;
    falling_piece_y_pos = -2;
    hit_floor = false;
    rotation = 0;
    current_peice = mi_random() % (uint16_t)NUMBER_PIECES;

    move_left = false;
    move_right = false;
    rotate_cw = false;
    rotate_ccw = false;
    fast_drop = false;
    gravity = 1;

    for (uint8_t i; i < 8; i++)
    {
        field_matrix[i] = {0};
    }
    set_servo_angle(90);
}

void update_matrix( bool show_piece)
{
    uint8_t i,j;
    uint8_t temp_matrix[8];
    for ( j = 0; j < 8; j++)
    {
        temp_matrix[j] = field_matrix[j];
    }

    //put piece on board
    if(show_piece)
    {
        for (i = 0; i < 3; i++)
        {
            for (j = 0; j < 3; j++)
            {
                if (piece_array[current_peice][rotation][i][j])
                {
                    if ((falling_piece_x_pos + j) < 8 &&
                        (falling_piece_x_pos + j) > -1 &&
                        (falling_piece_y_pos + i) < 8 &&
                        (falling_piece_y_pos + i) > -1)
                    {
                        temp_matrix[falling_piece_y_pos+i] |= (1<<(j+falling_piece_x_pos));
                    }
                }
            }
        }
    }

    uint8_t out[8] = {0};
    //rotate for gravity change
    switch (gravity)
    {
        case 0: //left
        {
            for (i = 0; i < 8; ++i)
            {
                for (j = 0; j < 8; ++j) 
                {
                    uint8_t bit = (temp_matrix[j] >> i) & 1;
                    out[i] |= bit << (7 - j);
                }
                WriteData ( i + 1, out[i]); 
            }
        }
        break;

        case 1: //regular
        {
            for ( i = 0; i < 8; i++)
            {
                WriteData ( i + 1, temp_matrix[i]); 
            }
        }
        break;

        case 2: //right
        {
            for (i = 0; i < 8; ++i) 
            {
                for (j = 0; j < 8; ++j) 
                {
                    uint8_t bit = (temp_matrix[j] >> (7-i)) & 1;
                    out[i] |= bit << j;
                }
                WriteData ( i + 1, out[i]); 
            }
        }
        break;
    }
    
    // for (int i = 0; i < 8; i++)
    // {
    //     Serial.println(temp_matrix[i]);
    // }
    //Serial.println();
}

void write_L()
{
    for ( uint8_t j = 0; j < 8; j++)
    {
        WriteData ( j + 1, L[j]); 
    }
}

void sand_animation() //called while sand
{
    bool still_sanding = false;

    clear_lines(false);

    for (int8_t i = 6; i >= 0; --i) //everything but bottom
    {
        for (int8_t j = 0; j < 8; ++j) 
        {
            if ( field_matrix[i] & (1<<j) )
            {
                if (!(field_matrix[i+1] &(1<<j))) //check right bellow piece
                {
                    field_matrix[i] &= ~(1<<j);
                    field_matrix[i+1] |= (1<<j);
                    still_sanding = true;
                }
                else if ( j < 7 && !(field_matrix[i+1] & (1<<(j+1))) ) //check to the right and bellow piece
                {
                    field_matrix[i] &= ~(1<<j);
                    field_matrix[i+1] |= (1<<(j+1));
                    still_sanding = true;
                }
                else if ( j > 0 && !(field_matrix[i+1] & (1<<(j-1))) ) //check to the left and bellow piece
                {
                    field_matrix[i] &= ~(1<<j);
                    field_matrix[i+1] |= (1<<(j-1));
                    still_sanding = true;
                }
            }
        }
    }

    sand = still_sanding;
}

void change_gravity()
{
    Serial.println("gravity change");
    uint8_t i, j, bit;
    uint8_t old_gravity = gravity;
    int8_t dif;
    uint8_t temp_matrix[8] = {0};

    gravity = (gravity + (mi_random() % 2 + 1)) % 3; //maske random (plus 1 or 2)

    set_servo_angle(90*gravity);

    dif = old_gravity - gravity;
    while (dif != 0)
    {
        if ( dif < 0 )
        {
            for (i = 0; i < 8; ++i) 
            {
                temp_matrix[i] = 0;
                for (j = 0; j < 8; ++j) 
                {
                    bit = (field_matrix[j] >> i) & 1;
                    temp_matrix[i] |= bit << (7 - j);
                }
            }
            dif++;
        }
        else
        {
            Serial.println("rotate");
            for (i = 0; i < 8; ++i) 
            {
                temp_matrix[i] = 0;
                for (j = 0; j < 8; ++j) 
                {
                    bit = field_matrix[j] >> (7-i) & 1;
                    temp_matrix[i] |= bit << j;
                }
            }
            dif--;
        }

        for ( i = 0; i < 8; i++)
        {
            field_matrix[i] = temp_matrix[i];
        }
    }
    
    sand = true;
}

bool check_hit()
{
    bool retval = false;

    for( uint8_t i = 0; i < 3; i++)
    {
        for ( uint8_t j = 0; j < 3; j++)
        {
            if (piece_array[current_peice][rotation][i][j])
            {
                if((falling_piece_x_pos + j) > 7 || (falling_piece_x_pos + j) < 0 )
                {
                    retval = true;
                }

                else if((falling_piece_y_pos + i) > 7)
                {
                    retval = true;
                    hit_floor = true;
                }

                else if((falling_piece_x_pos + j) < 8 &&
                        (falling_piece_x_pos + j) > -1 &&
                        (falling_piece_y_pos + i) < 8 &&
                        (falling_piece_y_pos + i) > -1 &&
                        (field_matrix[falling_piece_y_pos+i] & (uint8_t)(1<<(j+falling_piece_x_pos))))
                {
                    retval = true;
                    hit_floor = true;
                    //Serial.print(field_matrix[falling_piece_y_pos+i]);
                    Serial.println(" hit piece");
                }
            }
        }
    }

    return retval;
}

bool check_input()
{
    bool input_happened = false;
    move_left = false;
    move_right = false;
    fast_drop = false;
    rotate_ccw = false;
    //int xValue = 500;
    //int yValue = 500;

    int xValue = analogReadManual(VRX_PIN);
    int yValue = analogReadManual(VRY_PIN);
    // if(Serial.available())
    // {
    //     char key = Serial.read();
    //     Serial.println(key);
    //     switch (key)
    //     {
    //     case 'w':
    //     case 'W':
    //     {
    //         yValue = 0;
    //     }
    //     break;
    //     case 's':
    //     case 'S':
    //     {
    //         yValue = 1000;
    //     }
    //     break;
    //     case 'a':
    //     case 'A':
    //     {
    //         xValue = 0;
    //     }
    //     break;
    //     case 'd':
    //     case 'D':
    //     {
    //         xValue = 1000;
    //     }
    //     break;
        
    //     default:
    //         break;
    //     }
    // }
    //Serial.print(xValue);
    //Serial.print(", ");
    //Serial.println(yValue);

    if (yValue < 200) 
    { 
        rotate_ccw = true;
    } // Up
    if (yValue > 900) 
    { 
        fast_drop = true;
    }  // Down
    if (xValue < 200) 
    { 
        move_left = true;
        
    } // Left
    if (xValue > 800) 
    { 
        move_right = true;
    }  // Right

    input_happened = (move_left || move_right || rotate_ccw || rotate_cw || fast_drop);
    
    return input_happened;
}

void update_piece()
{
    if (fast_drop)
    {
        while (! (check_hit() & hit_floor))
        {
            falling_piece_y_pos += 1;
        }
        falling_piece_y_pos -=1 ;
        TCNT5 = OCR5A - 10; //reset timer 5 to top for snappyness

    }
    else
    {
        if (move_right)
        {
            falling_piece_x_pos -= 1;
            if(check_hit())
            {
                falling_piece_x_pos += 1;
            }
        }
        else if (move_left)
        {
            falling_piece_x_pos += 1;
            if(check_hit())
            {
                falling_piece_x_pos -= 1;
            }
        }
        // else if (rotate_cw)
        // {
        //     rotation += 1;
        //     if (check_hit())
        //     {
        //         rotation -= 1;
        //     }
        //     rotate_cw = false;
        // }
        else if (rotate_ccw)
        {
            rotation = (uint8_t)(rotation-1) % 4;
            //Serial.println(rotation);
            if (check_hit())
            {
                rotation = (rotation+1) % 4;
            }
        }
    }
}

bool fall_piece()
{   
    bool off_screen = false;
    falling_piece_y_pos += 1;
    Serial.println(F("fall_piece"));
    if (check_hit() & hit_floor)
    {
        Serial.println(".................................");
        falling_piece_y_pos -= 1;
        //check piece off screen
        for( uint8_t i = 0; i < 3; i++)
        {
            for ( uint8_t j = 0; j < 3; j++)
            {
                if (piece_array[current_peice][rotation][i][j])
                {
                    if((falling_piece_y_pos + i) < 0)
                    {
                        off_screen = true;
                    }
                }
            }
        }

        if (off_screen)
        {
            return false;
        }
        else
        {
            new_piece();
            clear_lines(true);
        }
    }
    return true;
}

void clear_lines(bool gravity_change)
{
    bool cleared = false;

    for (int8_t j = 7; j >= 0; j--)
    {
        if (field_matrix[j] == 255)
        {
            if( j != 0)
            {
                for(uint8_t i = j; i >= 1; i--) //copy down rows
                {
                    field_matrix[i] = field_matrix[i-1];
                }
            }
            cleared = true;
            field_matrix[0] = 0;
            j = 8; //will be 7 after -1
        }
    }
    if(cleared && gravity_change)
    {
        change_gravity();
        OCR5A *= 0.9;  //speed up / difficulty
        TCNT5 = 0;
        Serial.println(OCR5A);
    }
}

void new_piece()
{
    uint8_t i,j;
    Serial.println("new_piece");
    //add piece to array
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            if (piece_array[current_peice][rotation][i][j])
            {
                field_matrix[falling_piece_y_pos+i] |= (1<<(j+falling_piece_x_pos));
            }
        }
    }

    falling_piece_x_pos = 2;
    falling_piece_y_pos = -2;
    hit_floor = false;
    rotation = 0;
    current_peice = mi_random() % uint16_t(NUMBER_PIECES);
}

//joystick
uint16_t analogReadManual(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // Select ADC channel
    ADCSRA |= (1 << ADSC);                    // Start conversion
    while (ADCSRA & (1 << ADSC));             // Wait for conversion to complete
    //Serial.println(ADC);
    return ADC;
}

void game_clks_init()
{
    //timer 5
    TCCR5B |= (1 << WGM52);
    // Prescaler 256
    TCCR5B |= (1 << CS52);
    OCR5A = 62499; //1000 ms
    // Enable Timer1 compare interrupt
    TIMSK5 |= (1 << OCIE5A);


    //timer 3
    TCCR3B |= (1 << WGM32);

    // Prescaler 64
    TCCR3B |= (1 << CS31) | (1 << CS30);

    OCR3A = 2499; // 10ms = 2500 counts

    // Enable Timer3 compare interrupt
    TIMSK3 |= (1 << OCIE3A);
}

//servo stuff
void pwm_init_servo() {
    DDRB |= (1 << PB5);

    //Timer1 Fast PWM
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM13) | (1 << WGM12);

    TCCR1A |= (1 << COM1A1);

    TCCR1B |= (1 << CS11);

    ICR1 = 40000;
}

void ADC_init()
{
    ADMUX = (1 << REFS0);
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}

void set_servo_angle(uint8_t angle) 
{
    if (angle > 180) angle = 180;

    uint16_t ticks = 1000 + ((uint32_t)angle * 4100) / 180;

    OCR1A = ticks;
}


//1 second game clk
ISR(TIMER5_COMPA_vect)
{
    if (!sand)
    {
        switch(state)
        {
            case PLAYING:
            {
                //Serial.println("fall");
                if(!fall_piece())
                {
                    state = LOST;
                    reset();
                }
                update_matrix(true);
                //Serial.println("matrix");
            }
            break;

            case LOST:
            {
                if(lost_count < 3)
                {
                    lost_count++;
                }
                Serial.println(F("you suk"));
                write_L();
            }
            break;
        }
    }
    else
    {

    }
}

//10 ms second game clk
ISR(TIMER3_COMPA_vect)
{
    static bool first = true;
    static uint8_t count = 0;

    if (!sand)
    {
        if (check_input())
        {
            if (first)
            {
                switch(state)
                {
                    case PLAYING:
                    {
                        update_piece();
                        //Serial.println("update_piece");
                        update_matrix(true);
                        //Serial.println("matrix");
                    }
                    break;

                    case LOST:
                    {
                        if (lost_count > 2)
                        {
                            reset();
                            TCNT5 = 0; //reset timer 5 for consitentcy
                            state = PLAYING;
                            lost_count = 0;
                        }
                    }
                    break;
                }
            }
            first = false;
        }
        else
        {
            first = true;
        }
        count = 0;
    }
    else
    {
        if( count > 50 )
        {
            sand_animation();
            update_matrix(false);
            count = 0;
        }
        count++;
    }
    
}
    