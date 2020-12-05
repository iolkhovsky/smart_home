#!/bin/bash
curl -d '{"led_r":"255", "led_g":"255", "led_b":"255"}' -H "Content-Type: application/json" -X POST http://localhost:8000/status
