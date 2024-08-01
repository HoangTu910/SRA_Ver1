import React, { useState } from "react";
import loginAnimation from '../../lotties/login-animation.json';
import Lottie from 'react-lottie';
import { signInWithEmailAndPassword, signInWithPopup, GoogleAuthProvider } from 'firebase/auth';
import { auth } from '../../config/firebase';
import { useNavigate } from 'react-router-dom';
import axios from "axios";
// @components
import {
  Card,
  Input,
  Button,
  CardBody,
  CardHeader,
  Typography,
} from "@material-tailwind/react";

function Login() {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [error, setError] = useState('');
  const [success, setSuccess] = useState('');
  const navigate = useNavigate(); // Initialize navigate function

  const handleEmailLogin = async () => {
    try {
        // Sign in with Firebase Authentication
        const userCredential = await signInWithEmailAndPassword(auth, email, password);
        const idToken = await userCredential.user.getIdToken();
        
        console.log('Token:', idToken); // Log token for debugging
        
        // Prepare payload for backend API
        const loginPayload = {
            email, password,
            token: idToken
        };
        console.log('Login payload:', loginPayload);

        // Make a request to the backend server
        const response = await axios.post(
          "http://113.161.225.11:6969/api/user/login",
          { email, password, token: idToken },
          {
            headers: {
              'Content-Type': 'application/json',
            },
          }
        );

        // Check if the response was successful
        if (response.status === 200) {
            const result = response.data;
            
            // Store token or session information
            localStorage.setItem('authToken', result.token);
            console.log('Login successful');
            
            setSuccess(true); // Set success state to true
            navigate('/'); // Navigate to dashboard on successful login
        } else {
            // Handle unsuccessful responses
            console.error('Login failed:', response.data.message || 'Unknown error');
            setError('Login failed. Please try again.'); // Set error state
            setSuccess(false); // Set success state to false
        }
    } catch (error) {
        console.error('Error signing in with email and password:', error);
        setError('Invalid email or password.'); 
        setSuccess(false); 
    }
  };
  
  

  const handleGoogleLogin = async () => {
    const provider = new GoogleAuthProvider();
    try {
      setError(''); // Clear previous errors
      setSuccess('');
      const result = await signInWithPopup(auth, provider);
      const token = await result.user.getIdToken();
      await fetch('http://localhost:6969/api/user/login', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ token }),
      });
      setSuccess('Login successful!');
    } catch (error) {
      console.error('Error signing in with Google:', error);
      setError('Error signing in with Google');
    }
  };

  const defaultOptions = {
    loop: true,
    autoplay: true,
    animationData: loginAnimation,
    rendererSettings: {
      preserveAspectRatio: "xMidYMid slice"
    }
  };

  // Conditional class names for input error styling
  const inputErrorClass = error ? 'border-red-500' : 'border-t-blue-gray-200';
  const inputSuccessClass = success ? 'border-green-500' : 'border-t-blue-gray-200';
  return (
    <section className="px-8" style={{ backgroundImage: 'url(../src/image/bg8.jpg)', backgroundSize: 'cover' }}>
      <div className="absolute inset-0 bg-black opacity-40" style={{ zIndex: 0 }}></div>
      <div className="container mx-auto h-screen grid place-items-center">
        <Card
          shadow={true}
          className="md:px-24 md:80vh py-auto border border-gray-300 bg-gray-50 rounded-[50px]"
        >
          <CardHeader shadow={false} floated={false} className="text-center pt-7 bg-gray-50">
            <Typography className="!text-blue-400 text-[28px] font-bold md:max-w-sm">
              Digital Healthcare
            </Typography>
            <div className="mb-4">
              <Lottie
                options={defaultOptions}
                height={150}
                width={200}
              />
            </div>
            <Typography className="!text-gray-600 text-[17px] font-normal md:max-w-sm">
              Here we help you take care of your health more conveniently with modern technology.
            </Typography>
          </CardHeader>
          <CardBody>
            <form
              action="#"
              className="flex flex-col gap-4"
            >
              <div>
                <label htmlFor="email">
                  <Typography
                    variant="small"
                    color="blue-gray"
                    className="block font-medium mb-2"
                  >
                    Your Email
                  </Typography>
                </label>
                <Input
                  id="email"
                  color="gray"
                  size="lg"
                  type="email"
                  name="email"
                  placeholder="name@mail.com"
                  className={`w-full placeholder:opacity-100 focus:border-t-primary ${inputErrorClass}`}
                  labelProps={{
                    className: "hidden",
                  }}
                  value={email}
                  onChange={(e) => setEmail(e.target.value)}
                />
              </div>
              <div>
                <label htmlFor="password">
                  <Typography
                    variant="small"
                    color="blue-gray"
                    className="block font-medium mb-2"
                  >
                    Your Password
                  </Typography>
                </label>
                <Input
                  id="password"
                  color="gray"
                  size="lg"
                  type="password" // Changed to 'password'
                  name="password"
                  placeholder="password"
                  className={`w-full placeholder:opacity-100 focus:border-t-primary ${inputErrorClass}`}
                  labelProps={{
                    className: "hidden",
                  }}
                  value={password}
                  onChange={(e) => setPassword(e.target.value)}
                />
              </div>
              {error && (
                <Typography
                  variant="small"
                  className="text-red-500 text-center"
                >
                  {error}
                </Typography>
              )}
              <Button size="lg" color="blue" fullWidth onClick={handleEmailLogin}>
                Continue
              </Button>
              <Button
                variant="outlined"
                size="lg"
                className="flex h-12 border-blue-gray-200 items-center justify-center gap-2"
                fullWidth
                onClick={handleGoogleLogin}
              >
                <img
                  src={`https://www.material-tailwind.com/logos/logo-google.png`}
                  alt="google"
                  className="h-6 w-6"
                />{" "}
                Sign in with Google
              </Button>

              <Typography
                variant="small"
                className="text-center mx-auto max-w-[19rem] !font-medium !text-gray-600"
              >
                Upon signing in, you consent to abide by our{" "}
                <a href="#" className="text-gray-900">
                  Terms of Service
                </a>{" "}
                &{" "}
                <a href="#" className="text-gray-900">
                  Privacy Policy.
                </a>
              </Typography>
            </form>
          </CardBody>
        </Card>
      </div>
    </section>
  );
}

export default Login;
