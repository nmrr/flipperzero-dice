// CC0 1.0 Universal (CC0 1.0)
// Public Domain Dedication
// https://github.com/nmrr


#include <stdio.h>
#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include <furi_hal_random.h>

typedef enum {
    EventTypeInput,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} EventApp;

static uint8_t dice = 0;
static uint8_t state = 0;

static void draw_callback(Canvas* canvas, void* ctx) 
{
    UNUSED(ctx);
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%u", dice);

    //canvas_draw_frame(canvas, 0, 0, 128, 64);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignBottom, "Flipper Dice");

    canvas_set_font(canvas, FontBigNumbers);

    if (state == 4) canvas_draw_str_aligned(canvas, 64, 32+20, AlignCenter, AlignBottom, buffer);
    else if (state >= 1)
    {
        switch(state) 
        {
            case 1  :
                canvas_draw_str_aligned(canvas, 64, 32+17, AlignCenter, AlignBottom, ".");
                break;
            case 2  :
                canvas_draw_str_aligned(canvas, 64, 32+17, AlignCenter, AlignBottom, "..");
                break;
            default  :
                canvas_draw_str_aligned(canvas, 64, 32+17, AlignCenter, AlignBottom, "...");
                break;
        }

        if (state == 3)
        {
            uint8_t randomuint8[1];
            while(1)
            {
                furi_hal_random_fill_buf(randomuint8,1);
                randomuint8[0] &= 0b00000111;                
                if (randomuint8[0] >= 1 && randomuint8[0] <= 6) break;
            }
            dice = randomuint8[0];
        }

        state++;
    }
}

static void input_callback(InputEvent* input_event, void* ctx) 
{
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    EventApp event = {.type = EventTypeInput, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

int32_t flipper_dice_app() 
{
    EventApp event;
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(EventApp));

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);

    while(1) 
    {
        furi_check(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk);

        if(event.type == EventTypeInput) 
        {
            if(event.input.key == InputKeyBack) 
            {
                break;
            }
            else if (event.input.type == InputTypeShort)
            {
                if (state == 0 || state == 4)
                {
                    state = 1;
                    notification_message(notification, &sequence_blink_magenta_100);
                }
            }
        } 
    }

    furi_message_queue_free(event_queue);

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    furi_record_close(RECORD_NOTIFICATION);

    return 0;
}
