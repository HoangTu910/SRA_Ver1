import React from 'react';
import { Input, Button } from '@material-tailwind/react';
import { auth } from '../../../config/firebase'
import axios from 'axios';

export default function InputWithButton() {
  const [deviceId, setDeviceId] = React.useState(""); // Device ID input
  const [loading, setLoading] = React.useState(false);
  const [error, setError] = React.useState("");

  const onChange = ({ target }) => setDeviceId(target.value);

  const handleConfirm = async () => {
    if (!deviceId) return;

    setLoading(true);
    setError("");
    var user = auth.currentUser;
    const userId = user.uid;
    try {
      const response = await axios.post('http://113.161.225.11:6969/api/devices/upload', { deviceId });
      const responseForIdDeviceInUser = await axios.post('http://113.161.225.11:6969/api/devices/userupload', { deviceId, userId })
      if (response.status === 200 && responseForIdDeviceInUser.status === 200) {
        console.log("Document created successfully!");
        alert("Document created successfully!");
        setDeviceId(""); // Clear input after success
      } else {
        throw new Error("Unexpected response status");
      }
    } catch (err) {
      console.error("Error creating document: ", err);
      setError("Error creating document. Please try again.");
    } finally {
      setLoading(false);
    }
  };


  return (
    <div className="relative flex w-full max-w-[24rem]">
      <Input
        label="Device ID"
        value={deviceId}
        onChange={onChange}
        className="pr-20"
        containerProps={{
          className: "min-w-0",
        }}
        type="text" // Use type text for general input
      />
      <Button
        size="sm"
        color={deviceId ? "gray" : "blue-gray"}
        disabled={!deviceId || loading}
        onClick={handleConfirm}
        className="!absolute right-1 top-1 rounded"
      >
        {loading ? 'Creating...' : 'Confirm'}
      </Button>
      {error && <p className="text-red-500 text-xs mt-2">{error}</p>}
    </div>
  );
}
