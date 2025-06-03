package hola.mqtt;
import org.eclipse.paho.client.mqttv3.*;
import java.util.Scanner;
public class Configurador {

    public static void main(String[] args) {
    	System.out.println("Configurador");
    	Scanner sc = new Scanner(System.in);
        String broker = "tcp://broker.emqx.io:1883";
        String clientId = "Cofigurador";
        String topic = "crypto/config";
        int qos = 1;
        double valor;
        String msg;
        MqttMessage message;

        try {
            MqttClient client = new MqttClient(broker, clientId);
            MqttConnectOptions options = new MqttConnectOptions();
            client.connect(options);

            if (client.isConnected()) {
                // Callback para manejar eventos
                client.setCallback(new MqttCallback() {
                    public void messageArrived(String topic, MqttMessage message) throws Exception {
                    }

                    public void connectionLost(Throwable cause) {
                        System.out.println("⚠ Conexión perdida: " + cause.getMessage());
                    }

                    public void deliveryComplete(IMqttDeliveryToken token) {
                        System.out.println("✅ Mensaje enviado");
                    }
                });

                // Suscribirse al topic
                client.subscribe(topic, qos);
                
                while (true) {
                	System.out.println("Introducir valor a superar:");
                	valor=sc.nextDouble();
                	msg= String.valueOf(valor);
                	message = new MqttMessage(msg.getBytes());
                    message.setQos(qos);
                    client.publish(topic, message);
                    Thread.sleep(100);
                }            
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

