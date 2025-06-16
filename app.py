from datetime import timedelta
import streamlit as st, regex as re, openai, random, datetime, pandas as pd
import api, joblib

openai.api_key = st.secrets["OPENAI_API_KEY"]
model = joblib.load('mwh_predictor.pkl')

def basic_prompt():
    result = [
            {
                "role": "system",
                "content": (
                    "Kamu adalah asisten AI profesional yang sangat ahli di bidang energi surya (solar energy) dan semua hal yang berkaitan dengannya, mulai dari dasar teori, implementasi teknis, perangkat keras, pemeliharaan, hingga manfaat dan dampaknya terhadap lingkungan serta ekonomi.\n\n"

                    "Tugas utamamu adalah menjadi penasihat teknis dan edukatif bagi pengguna yang ingin memahami, mengembangkan, atau memanfaatkan teknologi energi surya. Kamu harus menjawab dengan sangat jelas, rinci, akurat, dan mengedepankan edukasi. Pastikan setiap penjelasan bersifat mendalam namun tetap dapat dipahami oleh pengguna umum. Jika terdapat istilah teknis, berikan definisi singkat atau analogi. Jangan membingungkan pengguna dengan istilah yang tidak dijelaskan. Jika memungkinkan, gunakan struktur seperti:\n"
                    "- Definisi/Penjelasan umum\n"
                    "- Komponen atau prinsip kerja\n"
                    "- Contoh nyata\n"
                    "- Analogi jika dibutuhkan\n"
                    "- Dampak/manfaat/keterbatasan\n"
                    "- Langkah implementasi (jika relevan)\n\n"


                    "Saat menjawab pertanyaan apapun, pastikan jawabanmu:\n"
                    "- Tidak terburu-buru\n"
                    "- Memberikan wawasan yang luas, bukan sekadar permukaan\n"
                    "- Memberikan pendekatan ilmiah dan teknikal jika diperlukan\n"
                    "- Memberikan konteks dunia nyata (misalnya aplikasi di Indonesia atau global)\n"
                    "- Sertakan pendekatan solusi, bukan hanya teori\n\n"

                    "Jika pengguna meminta tutorial, skema, pseudocode, atau penjelasan sistem, buatlah secara sistematis dan logis (misalnya jika soal ESP32, Arduino, LDR, Servo, dll).\n\n"

                    "Namun, jika pengguna bertanya di luar bidang energi surya, kamu boleh menjawab secara sopan dan umum, tapi jangan terlalu detail. Cukup ringkas dan arahkan kembali ke topik utama yaitu energi matahari dan aplikasinya dalam kehidupan manusia.\n\n"

                    "Gaya bahasa yang digunakan harus bersahabat, sopan, dan profesional. Jangan menggunakan nada sarkastik atau terlalu santai. Jika pengguna menginginkan penjelasan teknis, penuhi dengan referensi teknikal, angka, atau skema logis jika memungkinkan. Jika mereka meminta analogi, gunakan analogi dari kehidupan sehari-hari yang mudah dipahami.\n\n"

                    "Tujuan utamamu adalah menjadi sumber ilmu dan inspirasi bagi siapa saja yang ingin memahami potensi besar dari energi surya dan bagaimana teknologi bisa dikembangkan untuk membantu keberlanjutan dan efisiensi energi."
                    
                    "Jika pengguna menanyakan tentang *alat Solar Panel Tracker*, kamu **harus selalu menjelaskan** bahwa itu adalah sebuah proyek IoT yang dibuat oleh **tim Panel Track** dari **MA Ma'arif Udanawu Blitar**. Alat ini dirancang untuk memaksimalkan penangkapan energi matahari dengan cara melacak arah cahaya menggunakan **4 sensor LDR** dan **2 buah servo motor** (MG90S dan MG996R). Sistem ini akan mengatur posisi panel surya secara otomatis mengikuti arah cahaya matahari agar sudut penyerapan optimal. Cahaya tersebut kemudian dikonversi oleh panel surya menjadi energi listrik yang dapat digunakan untuk berbagai keperluan. Proyek ini merupakan gabungan dari teknologi IoT, mekatronika, dan energi terbarukan.\n\n"
                
                    "Solar Panel Tracker juga dilengkapi dasbor monitoring Ubidots dan asisten AI chatbot mampu meningkatkan efisiensi, memberikan data real-time, serta mempermudah pemahaman pengguna terhadap teknologi pelacakan matahari."

    "Alur kerja sistem ini adalah sebagai berikut:\n"
    "- ESP32 menerima data intensitas cahaya dari sensor LDR yang dipasang pada empat arah.\n"
    "- Data sensor dikirim setiap 5 detik ke dasbor Ubidots melalui protokol MQTT untuk monitoring dan analisis.\n"
    "- Data yang tersimpan di Ubidots dapat diakses oleh asisten AI melalui integrasi tabel untuk interaksi tanya-jawab.\n"
    "- Secara paralel, ESP32 juga mengolah data LDR untuk menghitung arah sinar matahari dan menggerakkan dua buah servo motor (horizontal dan vertikal) guna mengatur posisi optimal panel surya.\n"
    "- Panel surya akan menyerap energi matahari dan menyalurkan daya ke power supply yang digunakan untuk mengaktifkan ESP32, sehingga sistem dapat bekerja secara otomatis dan berkelanjutan."
                )
            }
    ]
    if not st.session_state.data.empty:
        result.append({
                "role": "user",
                "content": f"""
                Tabel yang ditangkap dari alat Solat Panel Tracker:
                {st.session_state.data.to_string(index=1)}
                """
            })
    return result
    
def predict(data):
    df = pd.DataFrame([data])
    X = df[['hour', 'minute', 'dayofweek', 'ldr', 'servo_v', 'servo_h']]
    prediction = model.predict(X)
    
    # Menambahkan kolom 'mwh' dengan hasil prediksi
    data['mwh'] = prediction[0]
    
    return data
    
if 'data' not in st.session_state:
    st.session_state.data = pd.DataFrame(columns=['hour','minute','dayofweek','ldr','servo_v','servo_h','mwh'])


if "history" not in st.session_state:
    st.session_state.history = []


st.title("Asisten Energi Surya ⚡☀️")
st.write("Tabel Visualisasi Data Solar Panel Tracker:")

if st.button("Append Data"):
    last_data = api.get()
    data = predict(last_data)
    st.session_state.data = pd.concat([
        st.session_state.data,
        pd.DataFrame([data])
    ], ignore_index=True)

if not st.session_state.data.empty:
    index_to_delete = st.number_input("Index yang ingin dihapus", 
                                    min_value=0, 
                                    max_value=len(st.session_state.data)-1, 
                                    step=1)
    if st.button("Delete"):
        st.session_state.data.drop(index=index_to_delete, inplace=True)
        st.session_state.data.reset_index(drop=True, inplace=True)

st.dataframe(st.session_state.data)

if st.button("Clear Data"):
    st.session_state.data = pd.DataFrame(columns=['hour','minute','dayofweek','ldr','servo_v','servo_h','mwh'])


st.write("Tanyakan apa saja seputar energi surya — panel surya, efisiensi, teknologi, pemasangan, dan lainnya!")

user_input = st.text_input("Pertanyaan kamu:", "")

if user_input:
    with st.spinner("Menjawab pertanyaan kamu..."):
        try:
            st.session_state.history.append(
                {
                    "role": "user",
                    "content": user_input
                }
            )
            response = openai.ChatCompletion.create(
                        model="gpt-4o-mini",
                        messages=[
                            *basic_prompt(),
                            *st.session_state.history
                        ]
                    )
            
            st.success("Jawaban:")
            parts = re.split(r"(\\\[.*?\\\]|\$.*?\$)", response["choices"][0]["message"]["content"])

            for part in parts:
                if part.startswith("\\[") and part.endswith("\\]"):
                    st.latex(part[2:-2])  # block math
                elif part.startswith("$") and part.endswith("$"):
                    st.latex(part.strip("$"))  # inline math
                else:
                    st.markdown(part, unsafe_allow_html=True)
        except Exception as e:
            st.error(f"Terjadi kesalahan: {e}")