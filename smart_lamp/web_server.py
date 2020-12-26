from fastapi import FastAPI
from fastapi.encoders import jsonable_encoder
from fastapi.responses import HTMLResponse, JSONResponse
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel

from controller_api.smart_lamp_controller import SmartLampController


class Item(BaseModel):
    led_r: int
    led_g: int
    led_b: int


try:
    lamp = SmartLampController()
except Exception as e:
    print(f"Exception occured: {e}")


lamp_state = {"r": 0, "g": 0, "b": 0}
app = FastAPI()
app.mount("/static", StaticFiles(directory="static"), name="static")


@app.get("/status", response_class=HTMLResponse)
async def root():
    json_compatible_item_data = jsonable_encoder(lamp_state)
    return JSONResponse(content=json_compatible_item_data)


@app.post("/status")
async def control(item: Item):
    lamp_state["r"] = item.led_r
    lamp_state["g"] = item.led_g
    lamp_state["b"] = item.led_b
    lamp.switch_lamp(lamp_state)
    json_compatible_item_data = jsonable_encoder(lamp_state)
    return JSONResponse(content=json_compatible_item_data)
