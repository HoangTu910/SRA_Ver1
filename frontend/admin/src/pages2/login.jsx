import React from "react";
import loginAnimation from '../lotties/login-animation.json';
import Lottie from 'react-lottie';
// @components
// zxxzczxxzc
import {
  Card,
  Input,
  Button,
  CardBody,
  CardHeader,
  Typography,
} from "@material-tailwind/react";

// @icons
import { CpuChipIcon } from "@heroicons/react/24/solid";
import avatarBorderColor from "@material-tailwind/react/theme/components/avatar/avatarBorderColor";

function Login() {
  const defaultOptions = {
    loop: true,
    autoplay: true,
    animationData: loginAnimation,
    rendererSettings: {
      preserveAspectRatio: "xMidYMid slice"
    }
  };
  const scroll = {
    overflow: 'hidden', 
    '::-webkit-scrollbar': {
      display: 'none'
    },
    '-ms-overflow-style': 'none',
    'scrollbar-width': 'none'
  };
  return (
    <section className="px-8" style={{ backgroundImage: 'url(../src/image/bg12.png)', backgroundSize: 'cover'}}>
      <div className="absolute inset-0 bg-gray-500 opacity-50" style={{ zIndex: 0 }}></div>
      <div className="container mx-auto h-screen grid place-items-center">
        <Card
          shadow={false}
          // style={{border: 'none'}}
          className="md:px-24 md:80vh py-auto border border-gray-300"
        >
          <CardHeader shadow={false} floated={false} className="text-center pt-7">
          <Typography className="!text-blue-400 text-[28px] font-bold md:max-w-sm">
              Smart Healthcare Platform
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
                  className="w-full placeholder:opacity-100 focus:border-t-primary border-t-blue-gray-200"
                  labelProps={{
                    className: "hidden",
                  }}
                />
              </div>
              <div>
                <label htmlFor="email">
                  <Typography
                    variant="small"
                    color="blue-gray"
                    className="block font-medium mb-2"
                  >
                    Your Password
                  </Typography>
                </label>
                <Input
                  id="email"
                  color="gray"
                  size="lg"
                  type="email"
                  name="email"
                  placeholder="password"
                  className="w-full placeholder:opacity-100 focus:border-t-primary border-t-blue-gray-200"
                  labelProps={{
                    className: "hidden",
                  }}
                />
              </div>
              <Button size="lg" color="blue" fullWidth>
                continue
              </Button>
              <Button
                variant="outlined"
                size="lg"
                className="flex h-12 border-blue-gray-200 items-center justify-center gap-2"
                fullWidth
              >
                <img
                  src={`https://www.material-tailwind.com/logos/logo-google.png`}
                  alt="google"
                  className="h-6 w-6"
                />{" "}
                sign in with google
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
