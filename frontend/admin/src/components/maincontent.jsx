import React from 'react';
import { Box, Typography, Grid, Paper } from '@mui/material';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';

const data = [
  { name: '12/20', bloodPressure: 120 },
  { name: '2/4', bloodPressure: 130 },
  { name: '14/8', bloodPressure: 110 },
  { name: '6/3', bloodPressure: 140 },
  { name: '16/8', bloodPressure: 125 },
  { name: '12/4', bloodPressure: 135 },
];

const MainContent = () => {
  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" gutterBottom>
        Hi, Gleb Kuznetsov, it seems you are in Perfect Body Shape
      </Typography>
      <Typography variant="subtitle1" gutterBottom>
        Your current meal
      </Typography>
      <Grid container spacing={3}>
        <Grid item xs={12}>
          <Paper sx={{ p: 2 }}>
            <Typography variant="h6">Blood Pressure</Typography>
            <ResponsiveContainer width="100%" height={200}>
              <LineChart data={data}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="name" />
                <YAxis />
                <Tooltip />
                <Legend />
                <Line type="monotone" dataKey="bloodPressure" stroke="#8884d8" activeDot={{ r: 8 }} />
              </LineChart>
            </ResponsiveContainer>
          </Paper>
        </Grid>
        <Grid item xs={12} md={6}>
          <Paper sx={{ p: 2 }}>
            <Typography variant="h6">Blood Pressure</Typography>
            <Typography variant="h4">141/90 mmHg</Typography>
          </Paper>
        </Grid>
        <Grid item xs={12} md={6}>
          <Paper sx={{ p: 2 }}>
            <Typography variant="h6">Body Temperature</Typography>
            <Typography variant="h4">37 C</Typography>
          </Paper>
        </Grid>
        <Grid item xs={12} md={6}>
          <Paper sx={{ p: 2 }}>
            <Typography variant="h6">Body Weight</Typography>
            <Typography variant="h4">80 kg</Typography>
          </Paper>
        </Grid>
        <Grid item xs={12} md={6}>
          <Paper sx={{ p: 2 }}>
            <Typography variant="h6">Blood Glucose</Typography>
            <Typography variant="h4">140 mg/dl</Typography>
          </Paper>
        </Grid>
        <Grid item xs={12} md={6}>
          <Paper sx={{ p: 2 }}>
            <Typography variant="h6">Respiration Rate</Typography>
            <Typography variant="h4">60 beats min</Typography>
          </Paper>
        </Grid>
      </Grid>
    </Box>
  );
};

export default MainContent;
