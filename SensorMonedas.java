package hola.mqtt;
import org.eclipse.paho.client.mqttv3.*;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import org.json.JSONObject;

public class SensorMonedas {

    private static final String BROKER = "tcp://broker.emqx.io:1883";
    private static final String CLIENT_ID = "SensorMonedasApp";
    private static final String TOPIC_BTC = "crypto/bitcoin";
    private static final String TOPIC_SOL = "crypto/solana";

    public static void main(String[] args) {
        try {
            MqttClient client = new MqttClient(BROKER, CLIENT_ID);
            MqttConnectOptions options = new MqttConnectOptions();
            options.setAutomaticReconnect(true);
            options.setCleanSession(true);
            client.connect(options);
            System.out.println("Conectado al broker MQTT");

            while (true) {
                double btcPrice = getPrice("BTCUSDT");
                double solPrice = getPrice("SOLUSDT");

                publishPrice(client, TOPIC_BTC, btcPrice);
                publishPrice(client, TOPIC_SOL, solPrice);

                Thread.sleep(500); // Espera 1 segundo entre publicaciones
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static double getPrice(String symbol) {
        try {
            String urlStr = "https://api.binance.com/api/v3/ticker/price?symbol=" + symbol;
            URL url = new URL(urlStr);
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setRequestMethod("GET");

            BufferedReader in = new BufferedReader(
                    new InputStreamReader(conn.getInputStream()));
            String inputLine;
            StringBuilder response = new StringBuilder();

            while ((inputLine = in.readLine()) != null) response.append(inputLine);
            in.close();

            JSONObject json = new JSONObject(response.toString());
            return Double.parseDouble(json.getString("price"));

        } catch (Exception e) {
            System.err.println("Error obteniendo precio de " + symbol);
            return 0.0;
        }
    }

    private static void publishPrice(MqttClient client, String topic, double price) throws MqttException {
        String payload = Double.toString(price);
        MqttMessage message = new MqttMessage(payload.getBytes());
        message.setQos(1);
        client.publish(topic, message);
        System.out.println("Publicado en " + topic + ": " + payload);
    }
}
