import streamlit as st
import openai

# Ganti dengan API key kamu
openai.api_key = "OPENAI_API_KEY"

st.title("Asisten Energi Surya ⚡☀️")
st.write("Tanyakan apa saja seputar energi surya — panel surya, efisiensi, teknologi, pemasangan, dan lainnya!")

user_input = st.text_input("Pertanyaan kamu:", "")

if user_input:
    with st.spinner("Menjawab pertanyaan kamu..."):
        try:
            response = openai.ChatCompletion.create(
                model="gpt-4o-mini",
                messages=[
                    {
                        "role": "system",
                        "content": (
                            "Kamu adalah asisten ahli di bidang energi surya. "
                            "Tugasmu adalah membantu pengguna memahami konsep, teknologi, pemasangan, dan penggunaan energi matahari. "
                            "Jawaban harus jelas, edukatif, dan jika perlu sertakan contoh atau analogi sederhana."
                        )
                    },
                    {"role": "user", "content": user_input}
                ]
            )
            st.success("Jawaban:")
            st.write(response["choices"][0]["message"]["content"])
        except Exception as e:
            st.error(f"Terjadi kesalahan: {e}")
