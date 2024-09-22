#!/bin/bash

# Start backend
(cd backend && npm start) &

# Start front-end
(cd frontend && cd admin && npm run dev) &

# Start broker
(cd broker && npm start) &

# Start socket
(cd socket && npm start) &

wait
