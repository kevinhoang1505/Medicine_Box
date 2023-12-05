package com.example.medicinebox;

import android.Manifest;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.app.NotificationCompat;
import androidx.fragment.app.Fragment;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * A simple {@link Fragment} subclass.
 * Use the {@link MedicineBoxFragment#newInstance} factory method to
 * create an instance of this fragment.
 */
public class MedicineBoxFragment extends Fragment {

    private static final String TAG = MedicineBoxFragment.class.getSimpleName();
    private static final int REQUEST_BLUETOOTH_SCAN_PERMISSION = 1;
    private static final String DEFAULT_NAME = "MedicineBox";
    private static final String IN_HANDLING_GET_MEDICINE = "1";
    private static final String IN_CHECKING_MEDICINE_SLOT = "2";
    private static final String CHANNEL_NAME = "MedicineBoxChannel";
    private static final String CHANNEL_DESCRIPTION = "This is my notification channel";
    BluetoothGattCharacteristic characteristicForNotify;
    BluetoothGattCharacteristic characteristicForWrite;
    BluetoothGatt bluetoothGatt;
    BluetoothLeScanner bluetoothLeScanner;
    View view;
    BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.d(TAG, "onConnectionStateChange: Connected");
                // Device is connected, you can now discover services.
                if (ActivityCompat.checkSelfPermission(requireContext(), Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                    Log.e(TAG, "onConnectionStateChange: don't have permission to connect");
                    return;
                }
                gatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                // Handle disconnection
                Log.d(TAG, "onConnectionStateChange: Disconnected");
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                // Retrieve the service UUIDs
                List<BluetoothGattService> services = gatt.getServices();
                for (BluetoothGattService service : services) {
                    UUID serviceUuid = service.getUuid();
                    Log.d(TAG, "Service UUID: " + serviceUuid.toString());
                    characteristicForNotify = service.getCharacteristic(UUID.fromString("9cf8e0c1-0cca-4303-b6e9-06608ed4ce24"));
                    characteristicForWrite = service.getCharacteristic(UUID.fromString("9cf8e0c2-0cca-4303-b6e9-06608ed4ce24"));
                    if (characteristicForNotify != null) {
                        Log.d(TAG, "onServicesDiscovered: ");
                        gatt.setCharacteristicNotification(characteristicForNotify, true);

                        // Set the CCCD (Client Characteristic Configuration Descriptor) to enable notifications
                        BluetoothGattDescriptor descriptor = characteristicForNotify.getDescriptor(UUID.fromString("9cf8e0c1-0cca-4303-b6e9-06608ed4ce24"));
                        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                        gatt.writeDescriptor(descriptor);
                    }
                }
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            // Handle the incoming notification data
            String data = characteristic.getStringValue(0);
            if (data.contains("export_slot")) {
                showNotificationForGetMedicine("Trạng thái tủ thuốc", String.valueOf(data.charAt(12)));
                Log.d(TAG, "onCharacteristicRead: check notification");
            } else if (data.contains("0")) {
                String realData = data.replaceAll("[\\[|,]", "");
                showNotificationForCheckingSlot("Trạng thái kiểm tra thuốc", String.valueOf(getPositionsOfZero(realData)));
            } else if (data.contains("msg:")) {
                getActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(requireContext(), data.substring(4), Toast.LENGTH_SHORT).show();
                    }
                });
            }
            String finalData = data.replaceAll("[\\[|,]", "");
            inputDataToSlot(finalData);
            Log.d(TAG, "onCharacteristicChanged: " + data);
            // Process the data as needed
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate: ");
        super.onCreate(savedInstanceState);
        bluetoothLeScanner = BluetoothAdapter.getDefaultAdapter().getBluetoothLeScanner();

        if (ActivityCompat.checkSelfPermission(requireContext(), Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
            Log.e(TAG, "onCreate: Don't have permission to scan");
            requestPermissions(new String[]{Manifest.permission.BLUETOOTH_SCAN}, REQUEST_BLUETOOTH_SCAN_PERMISSION);
        }
        bluetoothLeScanner.startScan(scanCallback);

    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_BLUETOOTH_SCAN_PERMISSION) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                // Permission was granted. You can now proceed with Bluetooth scanning.
            } else {
                // Permission was denied. Handle this case appropriately, such as showing a message to the user or explaining why the permission is needed.
                Log.e(TAG, "Permission denied for Bluetooth scanning");
            }
        }
    }

    private List<Integer> getPositionsOfZero(String numbers) {
        List<Integer> positions = new ArrayList<>();

        for (int i = 0; i < numbers.length(); i++) {
            if (numbers.charAt(i) == '0') {
                positions.add(i + 1);
            }
        }
        return positions;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        view = inflater.inflate(R.layout.fragment_medicine_box, container, false);
        EditText editText = view.findViewById(R.id.id_number);
        Button enterBtn = view.findViewById(R.id.enter_button);
        enterBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String inputData = editText.getText().toString();
                sendData(inputData);
            }
        });
        return view;
    }

    private void sendData(String data) {
        if (characteristicForWrite != null) {
            characteristicForWrite.setValue(data.getBytes());
            bluetoothGatt.writeCharacteristic(characteristicForWrite);
        }
    }

    private void showNotificationForGetMedicine(String title, String slot) {
        Context context = requireContext();
        // Create a notification manager
        NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        // Create a notification channel (required for API level 26 and above)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(IN_HANDLING_GET_MEDICINE, CHANNEL_NAME, NotificationManager.IMPORTANCE_DEFAULT);
            channel.setDescription(CHANNEL_DESCRIPTION);
            channel.enableLights(true);
            channel.setLightColor(Color.RED);
            channel.enableVibration(true);
            notificationManager.createNotificationChannel(channel);
        }

        // Build the notification
        NotificationCompat.Builder builder = new NotificationCompat.Builder(context, IN_HANDLING_GET_MEDICINE)
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setContentTitle(title)
                .setContentText("Đang lấy thuốc ở ngăn: " + slot)
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                .setAutoCancel(true);  // Automatically removes the notification when the user taps on it

        // Show the notification
        notificationManager.notify(/*notificationId*/ 1, builder.build());
    }

    private void showNotificationForCheckingSlot(String title, String slot) {
        Context context = requireContext();
        // Create a notification manager
        NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        // Create a notification channel (required for API level 26 and above)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(IN_CHECKING_MEDICINE_SLOT, CHANNEL_NAME, NotificationManager.IMPORTANCE_DEFAULT);
            channel.setDescription(CHANNEL_DESCRIPTION);
            channel.enableLights(true);
            channel.setLightColor(Color.RED);
            channel.enableVibration(true);
            notificationManager.createNotificationChannel(channel);
        }

        // Build the notification
        NotificationCompat.Builder builder = new NotificationCompat.Builder(context, IN_CHECKING_MEDICINE_SLOT)
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setContentTitle(title)
                .setContentText("Hết thuốc ở ngăn: " + slot)
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                .setAutoCancel(true);  // Automatically removes the notification when the user taps on it

        // Show the notification
        notificationManager.notify(/*notificationId*/ 1, builder.build());
    }

    public void inputDataToSlot(String numberOfMedicine) {
        inputDataForEachButton(R.id.slot_1, numberOfMedicine, 0);
        inputDataForEachButton(R.id.slot_2, numberOfMedicine, 1);
        inputDataForEachButton(R.id.slot_3, numberOfMedicine, 2);
        inputDataForEachButton(R.id.slot_4, numberOfMedicine, 3);
        inputDataForEachButton(R.id.slot_5, numberOfMedicine, 4);
        inputDataForEachButton(R.id.slot_6, numberOfMedicine, 5);
        inputDataForEachButton(R.id.slot_7, numberOfMedicine, 6);
        inputDataForEachButton(R.id.slot_8, numberOfMedicine, 7);
        inputDataForEachButton(R.id.slot_9, numberOfMedicine, 8);
    }

    public void inputDataForEachButton(int id, String number, int position) {
        Button slot = view.findViewById(id);
        slot.setText(String.valueOf(number.charAt(position)));
    }

    ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            // Handle scan results here.
            BluetoothDevice device = result.getDevice();
            Log.d(TAG, "onScanResult: " + result.getDevice().getName());
            if (ActivityCompat.checkSelfPermission(requireContext(), Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                Log.e(TAG, "onScanResult: don't have permission to connect");
                return;
            }
            if (DEFAULT_NAME.equals(device.getName())) {
                bluetoothGatt = device.connectGatt(requireContext(), true, gattCallback);
                bluetoothLeScanner.stopScan(scanCallback);
            }
        }

        @Override
        public void onBatchScanResults(List<ScanResult> results) {
            super.onBatchScanResults(results);
            // Handle batch scan results
        }

        @Override
        public void onScanFailed(int errorCode) {
            super.onScanFailed(errorCode);
            // Handle scan failure
        }
    };
}